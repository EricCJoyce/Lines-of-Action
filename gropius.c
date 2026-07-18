/*

sudo docker run --rm -v $(pwd):/src -u $(id -u):$(id -g) --mount type=bind,source=$(pwd),target=/home/src c-wasm emcc -Os -s STANDALONE_WASM -s EXPORTED_FUNCTIONS="['_getInputGameStateBuffer','_getInputMoveBuffer','_getOutputGameStateBuffer','_getOutputMovesBuffer','_sideToMove_eval','_isTerminal_eval','_makeMove_eval','_makeNullMove_eval','_evaluate_eval','_getMoves_eval']" -Wl,--no-entry "gropius.c" -o "eval.wasm"

*/

#include "gamestate.h"
#include "gropius.h"

#define SEE_SCORE_PAWN               10                             /* Static Exchange Evaluation, rough pawn score. */

/**************************************************************************************************
 Typedefs  */


/**************************************************************************************************
 Prototypes  */

unsigned char* getInputGameStateBuffer(void);
unsigned char* getInputMoveBuffer(void);
unsigned char* getOutputGameStateBuffer(void);
unsigned char* getOutputMovesBuffer(void);

void serializeGameStateToBuffer(GameState*, unsigned char*);
void serializeMoveToBuffer(Move*, unsigned char*);
void deserializeGameState(GameState*);
void deserializeMove(Move*);

unsigned char sideToMove_eval(void);
bool isTerminal_eval(void);
void makeMove_eval(void);
void makeNullMove_eval(void);
float evaluate_eval(void);
unsigned int getMoves_eval(void);
signed int SEE(Move*, GameState*);

/**************************************************************************************************
 Globals  */

unsigned char inputGameStateBuffer[_GAMESTATE_BYTE_SIZE];           //  Global array containing the serialized INPUT game state.

unsigned char inputMoveBuffer[_MOVE_BYTE_SIZE];                     //  Global array containing the serialized INPUT move.

unsigned char outputGameStateBuffer[_GAMESTATE_BYTE_SIZE];          //  Global array containing the serialized OUTPUT game state.

                                                                    //  Global array containing up to _MAX_MOVES moves.
                                                                    //  Rather than encode the number of moves in the array itself, we return an integer.
                                                                    //  Each move is represented as a byte sub-array encoding:
                                                                    //    _MOVE_BYTE_SIZE  :  bytes encoding a single move,
                                                                    //    4                :  bytes for signed integer, which is rough score.
unsigned char outputMovesBuffer[_MAX_MOVES * (_MOVE_BYTE_SIZE + 5)];//    1                :  byte (should be Boolean) indicating whether move is "quiet".

/**************************************************************************************************
 Functions  */

/* Expose the global array declared here to JavaScript.  */
unsigned char* getInputGameStateBuffer(void)
  {
    return &inputGameStateBuffer[0];
  }

/* Expose the global array declared here to JavaScript.  */
unsigned char* getInputMoveBuffer(void)
  {
    return &inputMoveBuffer[0];
  }

/* Expose the global array declared here to JavaScript.  */
unsigned char* getOutputGameStateBuffer(void)
  {
    return &outputGameStateBuffer[0];
  }

/* Expose the global array declared here to JavaScript.  */
unsigned char* getOutputMovesBuffer(void)
  {
    return &outputMovesBuffer[0];
  }

/* Write the given game state to the given buffer. */
void serializeGameStateToBuffer(GameState* gs, unsigned char* buffer)
  {
    unsigned char x, y;
    unsigned char i = 0;
    unsigned char ch, mask;

    for(y = 0; y < 8; y++)                                          //  (8 bytes) Encode black.
      {
        ch = 0;
        mask = 128;
        for(x = 0; x < 8; x++)
          {
            if(isBlack(y * 8 + x, gs))
              ch |= mask;
            mask >>= 1;
          }
        buffer[i++] = ch;
      }

    for(y = 0; y < 8; y++)                                          //  (8 bytes) Encode white.
      {
        ch = 0;
        mask = 128;
        for(x = 0; x < 8; x++)
          {
            if(isWhite(y * 8 + x, gs))
              ch |= mask;
            mask >>= 1;
          }
        buffer[i++] = ch;
      }

    buffer[i] = gs->blackToMove ? 128 : 0;                          //  (1 byte) Encode side to move.

    return;                                                         //  TOTAL: 17 bytes.
  }

/* Write the given move to the given buffer. */
void serializeMoveToBuffer(Move* move, unsigned char* buffer)
  {
    unsigned char i = 0;

    buffer[i++] = move->from;
    buffer[i++] = move->to;

    return;
  }

/* Recover a GameState from the unsigned-char buffer "inputGameStateBuffer". */
void deserializeGameState(GameState* gs)
  {
    unsigned char x, y;
    unsigned char i;
    unsigned char ch, mask;

    for(i = 0; i < _NONE; i++)                                      //  Fill-in/blank-out.
      gs->board[i] = _EMPTY;

    i = 0;

    for(y = 0; y < 8; y++)                                          //  (8 bytes) Decode black.
      {
        ch = inputGameStateBuffer[i++];
        mask = 128;
        for(x = 0; x < 8; x++)
          {
            if((ch & mask) == mask)
              gs->board[y * 8 + x] = _BLACK_PAWN;
            mask >>= 1;
          }
      }

    for(y = 0; y < 8; y++)                                          //  (8 bytes) Decode white.
      {
        ch = inputGameStateBuffer[i++];
        mask = 128;
        for(x = 0; x < 8; x++)
          {
            if((ch & mask) == mask)
              gs->board[y * 8 + x] = _WHITE_PAWN;
            mask >>= 1;
          }
      }

    gs->blackToMove = ((inputGameStateBuffer[i] & 128) == 128);     //  (1 byte) Decode side to move.

    return;                                                         //  TOTAL: 17 bytes.
  }

/* Recover a Move from the unsigned-char buffer "inputMoveBuffer". */
void deserializeMove(Move* move)
  {
    move->from = inputMoveBuffer[0];
    move->to = inputMoveBuffer[1];
    return;
  }

/* Answer the Negamax Module's query, "Which side is to move in the GameState in the query buffer?"
   Return an unsigned char in {_BLACK_TO_MOVE, _WHITE_TO_MOVE}. */
unsigned char sideToMove_eval(void)
  {
    GameState gs;
    deserializeGameState(&gs);                                      //  Recover GameState from buffer.
    return gs.blackToMove ? _BLACK_TO_MOVE : _WHITE_TO_MOVE;
  }

/* Answer the Negamax Module's query, "Is the GameState in the query buffer terminal?" */
bool isTerminal_eval(void)
  {
    GameState gs;
    deserializeGameState(&gs);                                      //  Recover GameState from buffer.
    return terminal(&gs);
  }

/* Answer the Negamax Module's query, "What GameState results from making the move in the input-move buffer in the game state in the input-gamestate buffer?"
   Writes to "outputGameStateBuffer". */
void makeMove_eval(void)
  {
    GameState gs;
    Move move;

    deserializeGameState(&gs);                                      //  Recover GameState from buffer.
    deserializeMove(&move);                                         //  Recover Move from buffer.

    makeMove(&move, &gs);                                           //  Make the move.

    serializeGameStateToBuffer(&gs, outputGameStateBuffer);         //  Write updated GameState to output-gamestate buffer.

    return;
  }

/* For use by null-move pruning in tree-search.
   Answer the Negamax Module's query, "What GameState results from a null-move in the game state in the input-gamestate buffer?"
   Writes to "outputGameStateBuffer". */
void makeNullMove_eval(void)
  {
    GameState gs;

    deserializeGameState(&gs);                                      //  Recover GameState from buffer.

    makeNullMove(&gs);                                              //  Make the move.

    serializeGameStateToBuffer(&gs, outputGameStateBuffer);         //  Write updated GameState to output-gamestate buffer.

    return;
  }

/* Answer the Negamax Module's query, "What is the evaluation of the GameState in the input-gamestate buffer?" */
float evaluate_eval(void)
  {
    GameState gs;
    deserializeGameState(&gs);                                      //  Recover GameState from buffer.
    return score(&gs);                                              //  Negamax rule: always evaluate for the side that is now to move.
  }

/* Answer the Negamax Module's query, "What are all the moves that can be made from the GameState in the input-gamestate buffer?"
   Writes to "outputMovesBuffer":
     [_MOVE_BYTE_SIZE bytes of move, 4 bytes of a signed int, 1 byte indicating whether the move is "quiet"],
     [_MOVE_BYTE_SIZE bytes of move, 4 bytes of a signed int, 1 byte indicating whether the move is "quiet"],
                                                         . . .
     [_MOVE_BYTE_SIZE bytes of move, 4 bytes of a signed int, 1 byte indicating whether the move is "quiet"] */
unsigned int getMoves_eval()
  {
    GameState gs, child;
    Move moves[_MAX_MOVES];
    unsigned int movesLen = 0;

    unsigned char blackMaterial_Prior[_TOTAL_PIECES];               //  Indices of all Black material, BEFORE.
    unsigned char blackMaterialLength_Prior = 0;
    unsigned char whiteMaterial_Prior[_TOTAL_PIECES];               //  Indices of all White material, BEFORE.
    unsigned char whiteMaterialLength_Prior = 0;

    unsigned char blackMaterial_Posterior[_TOTAL_PIECES];           //  Indices of all Black material, AFTER.
    unsigned char blackMaterialLength_Posterior = 0;
    unsigned char whiteMaterial_Posterior[_TOTAL_PIECES];           //  Indices of all White material, AFTER.
    unsigned char whiteMaterialLength_Posterior = 0;

    float concentration_Prior, concentration_Posterior;
    float connected_Prior, connected_Posterior;

    signed int score_j, scores[_MAX_MOVES];                         //  Use fast, cheap heuristics.

    unsigned char buffer4[4];                                       //  Byte array to hold leading int = number of moves in output buffer.
    unsigned int i, j, k;

    blackMaterialLength_Prior = getBlack(&gs, blackMaterial_Prior); //  unsigned chars
    whiteMaterialLength_Prior = getWhite(&gs, whiteMaterial_Prior); //  unsigned chars

    deserializeGameState(&gs);                                      //  Recover GameState from buffer.
    movesLen = getMoves(&gs, moves);                                //  Get moves.

    if(gs.blackToMove)                                              //  We will compare these before and after.
      {
        concentration_Prior = concentration(blackMaterial_Prior, blackMaterialLength_Prior) - concentration(whiteMaterial_Prior, whiteMaterialLength_Prior);
        connected_Prior = connectedness(blackMaterial_Prior, blackMaterialLength_Prior, &gs) - connectedness(whiteMaterial_Prior, whiteMaterialLength_Prior, &gs);
      }
    else
      {
        concentration_Prior = concentration(whiteMaterial_Prior, whiteMaterialLength_Prior) - concentration(blackMaterial_Prior, blackMaterialLength_Prior);
        connected_Prior = connectedness(whiteMaterial_Prior, whiteMaterialLength_Prior, &gs) - connectedness(blackMaterial_Prior, blackMaterialLength_Prior, &gs);
      }

    for(i = 0; i < movesLen; i++)                                   //  Compute a fast-n-cheap score to help the Negamax Module sort its nodes.
      {
        scores[i] = 0;                                              //  Initialize every move to zero.

        if(isCapture(moves + i, &gs))                               //  Move is a capture.
          scores[i] += SEE(moves + i, &gs) * 20;                    //  Static Exchange Evaluation can reveal good, equal, or bad captures.

        copyGameState(&gs, &child);                                 //  Clone the source state.
        makeMove(moves + i, &child);                                //  Apply the candidate move.
        if(terminal(&child))
          scores[i] += 10000;

        blackMaterialLength_Posterior = getBlack(&child, blackMaterial_Posterior);
        whiteMaterialLength_Posterior = getWhite(&child, whiteMaterial_Posterior);

        if(gs.blackToMove)                                          //  Compare before and after.
          {
            concentration_Posterior = concentration(blackMaterial_Posterior, blackMaterialLength_Posterior) - concentration(whiteMaterial_Posterior, whiteMaterialLength_Posterior);
            connected_Posterior = connectedness(blackMaterial_Posterior, blackMaterialLength_Posterior, &child) - connectedness(whiteMaterial_Posterior, whiteMaterialLength_Posterior, &child);
          }
        else
          {
            concentration_Posterior = concentration(whiteMaterial_Posterior, whiteMaterialLength_Posterior) - concentration(blackMaterial_Posterior, blackMaterialLength_Posterior);
            connected_Posterior = connectedness(whiteMaterial_Posterior, whiteMaterialLength_Posterior, &child) - connectedness(blackMaterial_Posterior, blackMaterialLength_Posterior, &child);
          }
        scores[i] += (signed int)round((concentration_Posterior - concentration_Prior) * 80.0);
        scores[i] += (signed int)round((connected_Posterior - connected_Prior) * 300.0);
      }

    i = 0;                                                          //  Point to head of output buffer.
    for(j = 0; j < movesLen; j++)                                   //  Write moves as bytes to output buffer, following uint total number of moves.
      {
        outputMovesBuffer[i++] = moves[j].from;                     //  Copy move to global byte array.
        outputMovesBuffer[i++] = moves[j].to;

        score_j = scores[j];
        memcpy(buffer4, (unsigned char*)(&score_j), 4);             //  Force the SIGNED integer into a 4-byte temp buffer.
        for(k = 0; k < 4; k++)                                      //  Copy local SIGNED score to global output byte array.
          outputMovesBuffer[i++] = buffer4[k];
                                                                    //  0: quiet; 1: capture.
        outputMovesBuffer[i++] = (!isCapture(moves + j, &gs)) ? 0 : 1;
      }

    return movesLen;
  }

/* Static Exchange Evaluation */
signed int SEE(Move* move, GameState* src)
  {
    GameState gs;
    signed int gains[_NONE];
    unsigned char gainsLen = 0;
    unsigned char target;
    signed int capturedPieceVal;
    char capturingPiece;
    char team;
    Move buffer[_TOTAL_PIECES];
    Move chosenMove;
    unsigned char len, i;
    signed int leastVal, val, victimVal, j;
    signed int stopHere, continueExchange;

    copyGameState(src, &gs);
    target = move->to;                                              //  Identify the target.

    capturedPieceVal = SEE_SCORE_PAWN;                              //  Value of the captured piece.
    capturingPiece = gs.board[move->from];
    gains[0] = capturedPieceVal;
    makeMove(move, &gs);                                            //  Apply the move to be evaluated.
    team = gs.blackToMove ? 'b' : 'w';

    while(true)                                                     //  Follow capture chain to the end.
      {
        len = attackersOfSquare(target, team, &gs, buffer);
        if(len == 0)                                                //  No further captures.
          break;

        leastVal = SEE_SCORE_PAWN;
        for(i = 0; i < len; i++)
          {
            val = SEE_SCORE_PAWN;
            if(val < leastVal)
              {
                chosenMove.from = buffer[i].from;
                chosenMove.to = buffer[i].to;
                leastVal = val;
              }
          }
        victimVal = SEE_SCORE_PAWN;                                 //  Value of the victim.
        gainsLen++;
        gains[gainsLen] = victimVal - gains[gainsLen - 1];

        capturingPiece = gs.board[chosenMove.from];                 //  Identify the capturing piece.
        makeMove(&chosenMove, &gs);                                 //  Make the capture.
        team = gs.blackToMove ? 'b' : 'w';
      }

    if(gainsLen > 0)
      {
        for(j = gainsLen - 1; j >= 0; j--)
          {
            stopHere = -gains[j];                                   //  It is in side-to-move's interest to stop here.
            continueExchange = gains[j + 1];                        //  It is in side-to-move's interest to continue exchanging.
            gains[j] = (stopHere > continueExchange) ? stopHere : continueExchange;
          }
      }

    return gains[0];
  }
