/*

Game logic module for the human player.

sudo docker run --rm -v $(pwd):/src -u $(id -u):$(id -g) --mount type=bind,source=$(pwd),target=/home/src emscripten-c emcc -Os -s STANDALONE_WASM -s EXPORTED_FUNCTIONS="['_getCurrentState','_getMovesBuffer','_sideToMove_client','_isBlack_client','_isWhite_client','_isEmpty_client','_isPawn_client','_isKnight_client','_isBishop_client','_isRook_client','_isQueen_client','_isKing_client','_whiteKingsidePrivilege_client','_whiteQueensidePrivilege_client','_whiteCastled_client','_blackKingsidePrivilege_client','_blackQueensidePrivilege_client','_blackCastled_client','_getMovesIndex_client','_makeMove_client','_isTerminal_client','_isWin_client','_draw']" -Wl,--no-entry "gamelogic.c" -o "gamelogic.wasm"

*/

#include "gamestate.h"

/**************************************************************************************************
 Typedefs  */


/**************************************************************************************************
 Prototypes  */

__attribute__((import_module("env"), import_name("_printRow"))) void printRow(char a, char b, char c, char d, char e, char f, char g, char h);
__attribute__((import_module("env"), import_name("_printGameStateData"))) void printGameStateData(bool bToMove);
unsigned char* getCurrentState(void);
unsigned char* getMovesBuffer(void);
void serialize(GameState*);
void deserialize(GameState*);

unsigned char sideToMove_client(void);
bool isBlack_client(unsigned char);
bool isWhite_client(unsigned char);
bool isEmpty_client(unsigned char);

unsigned int getMovesIndex_client(unsigned char);
void makeMove_client(unsigned char, unsigned char, unsigned char);
bool isTerminal_client(void);
unsigned char isWin_client(void);

void draw(void);

/**************************************************************************************************
 Globals  */

unsigned char currentState[_GAMESTATE_BYTE_SIZE];                   //  Global array containing the serialized game state.
unsigned char movesBuffer[_MAX_NUM_TARGETS];                        //  Global array containing the unique destination-indices
                                                                    //  (not necessarily the number of unique moves) available.

/**************************************************************************************************
 Functions  */

/* Expose the global array declared here to JavaScript.  */
unsigned char* getCurrentState(void)
  {
    return &currentState[0];
  }

/* Expose the global array declared here to JavaScript.  */
unsigned char* getMovesBuffer(void)
  {
    return &movesBuffer[0];
  }

/* Pack a GameState into the unsigned-char buffer "currentState". */
void serialize(GameState* gs)
  {
    unsigned char x, y;
    unsigned char i = 0;
    unsigned char ch;

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
        currentState[i++] = ch;
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
        currentState[i++] = ch;
      }

    currentState[i] = gs->blackToMove ? 128 : 0;                    //  (1 byte) Encode side to move.

    return;                                                         //  TOTAL: 17 bytes.
  }

/* Recover a GameState from the unsigned-char buffer "currentState". */
void deserialize(GameState* gs)
  {
    unsigned char x, y;
    unsigned char i = 0;
    unsigned char ch;

    for(y = 0; y < 8; y++)                                          //  (8 bytes) Decode black.
      {
        ch = currentState[i++];
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
        ch = currentState[i++];
        mask = 128;
        for(x = 0; x < 8; x++)
          {
            if((ch & mask) == mask)
              gs->board[y * 8 + x] = _WHITE_PAWN;
            mask >>= 1;
          }
      }

    gs->blackToMove = ((currentState[i] & 128) == 128);             //  (1 byte) Decode side to move.

    return;                                                         //  TOTAL: 17 bytes.
  }

/* Answer the client-side question, Whose turn is it? */
unsigned char sideToMove_client(void)
  {
    GameState gs;
    deserialize(&gs);                                               //  Recover GameState from buffer.
    return (gs.blackToMove) ? _BLACK_TO_MOVE : _WHITE_TO_MOVE;
  }

bool isBlack_client(unsigned char index)
  {
    GameState gs;
    deserialize(&gs);                                               //  Recover GameState from buffer.
    return isBlack(index, &gs);
  }

bool isWhite_client(unsigned char index)
  {
    GameState gs;
    deserialize(&gs);                                               //  Recover GameState from buffer.
    return isWhite(index, &gs);
  }

bool isEmpty_client(unsigned char index)
  {
    GameState gs;
    deserialize(&gs);                                               //  Recover GameState from buffer.
    return isEmpty(index, &gs);
  }

bool isTerminal_client(void)
  {
    GameState gs;
    deserialize(&gs);                                               //  Recover GameState from buffer.
    return terminal(&gs);
  }

/* Returns unsigned char in {GAME_ONGOING         = 0,
                             GAME_OVER_BLACK_WINS = 1,
                             GAME_OVER_WHITE_WINS = 2,
                             GAME_OVER_DRAW       = 3}. */
unsigned char isWin_client(void)
  {
    GameState gs;
    deserialize(&gs);                                               //  Recover GameState from buffer.
    return isWin(&gs);
  }

/* Given an index, recover the game state from the global buffer "currentState", and compute the moves available to the piece at "index."
   The number of moves is returned, and that many bytes in "movesBuffer" will contain a destinations.
   This function is intended to answer queries from the human player. */
unsigned int getMovesIndex_client(unsigned char index)
  {
    GameState gs;
    Move moves[_MAX_NUM_TARGETS];                                   //  Generous upper bound assumes that a single piece could reach half of all squares.
    unsigned int len, i = 0, j;
    unsigned int ctr;
    unsigned char indices[_MAX_NUM_TARGETS];

    deserialize(&gs);                                               //  Recover GameState from buffer.
    len = getMovesIndex(index, &gs, moves);

    ctr = 0;
    for(i = 0; i < len; i++)                                        //  Iterate through moves for index and identify unique destination indices.
      {
        j = 0;
        while(j < ctr && indices[j] != moves[i].to)
          j++;
        if(j == ctr)
          indices[ctr++] = moves[i].to;
      }

    i = 0;                                                          //  Reset. 'i' now iterates into 'movesBuffer'.
    for(len = 0; len < ctr; len++)
      movesBuffer[i++] = indices[len];

    return ctr;
  }

/* Update "currentState" according to the given move data (if those data are indeed valid!) */
void makeMove_client(unsigned char from, unsigned char to)
  {
    GameState gs;
    Move moves[_NONE];                                              //  Generous assumption that every square is reachable.
    Move move;
    unsigned int len, i;

    deserialize(&gs);                                               //  Recover GameState from buffer.
    len = getMovesIndex(from, &gs, moves);                          //  Make sure that this move is legal.
    i = 0;                                                          //  Otherwise, ignore it. Cheaters lose their turns!
    while(i < len && !(moves[i].from == from && moves[i].to == to))
      i++;
    if(i < len)
      {
        move.from = from;
        move.to = to;
        makeMove(&move, &gs);
      }

    serialize(&gs);                                                 //  Write updated GameState back to buffer.

    return;
  }

/* Draw the board to the JavaScript console.
   . B B B B B B .
   W . . . . . . W
   W . . . . . . W
   W . . . . . . W
   W . . . . . . W
   W . . . . . . W
   W . . . . . . W
   . B B B B B B . */
void draw(void)
  {
    GameState gs;
    signed char y;

    deserialize(&gs);                                               //  Recover GameState from buffer.

    for(y = 7; y >= 0; y--)
      printRow(gs.board[y * 8], gs.board[y * 8 + 1], gs.board[y * 8 + 2], gs.board[y * 8 + 3], gs.board[y * 8 + 4], gs.board[y * 8 + 5], gs.board[y * 8 + 6], gs.board[y * 8 + 7]);

    printGameStateData(gs.blackToMove);
    return;
  }
