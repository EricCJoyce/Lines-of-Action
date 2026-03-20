#ifndef __GAMESTATE_H
#define __GAMESTATE_H

#include <ctype.h>
#include <math.h>                                                   /* Needed for INFINITY. */
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>

#define _NONE                    64
#define _TOTAL_PIECES            12                                 /* At most, 12 pieces to a side. */

#define _EMPTY                 0x00
#define _BLACK_PAWN            0x01
#define _WHITE_PAWN            0x02

#define _BLACK_TO_MOVE            0
#define _WHITE_TO_MOVE            1

#define GAME_ONGOING              0
#define GAME_OVER_BLACK_WINS      1
#define GAME_OVER_WHITE_WINS      2
#define GAME_OVER_DRAW            3

#define _GAMESTATE_BYTE_SIZE     17                                 /* Number of bytes needed to store a GameState structure. */
#define _MOVE_BYTE_SIZE           2                                 /* Number of bytes needed to store a Move structure. */
#define _MAX_NUM_TARGETS         32                                 /* A (generous) upper bound on how many distinct destinations (not distinct moves)
                                                                       may be available to a player from a single index. */
#define _MAX_MOVES              128                                 /* A (generous) upper bound on how many moves are available to a team in a single turn. */

/**************************************************************************************************
 Typedefs  */

typedef struct GameStateType                                        //  TOTAL: 17 bytes.
  {
    bool blackToMove;                                               //  True: black to move. False: white to move.
    unsigned char board[_NONE];                                     //  Array of characters.
  } GameState;

typedef struct MoveType                                             //  TOTAL: 3 bytes.
  {
    unsigned char from;                                             //  Index in [0, 64).
    unsigned char to;                                               //  Index in [0, 64).
  } Move;

/**************************************************************************************************
 Prototypes  */

void copyGameState(GameState*, GameState*);

void makeMove(Move*, GameState*);
void makeNullMove(GameState*);
char nowToMove(GameState*);
char nextToMove(GameState*);
unsigned int getMoves(GameState*, Move*);
unsigned int getMovesIndex(unsigned char, GameState*, Move*);
bool isCapture(Move*, GameState*);
unsigned char columnMileage(unsigned char, GameState*);
unsigned char rowMileage(unsigned char, GameState*);
unsigned char forwardSlashMileage(unsigned char, GameState*);
unsigned char backSlashMileage(unsigned char, GameState*);

unsigned char isWin(GameState*);
bool terminal(GameState*);

bool isEmpty(unsigned char, GameState*);
bool isBlack(unsigned char, GameState*);
bool isWhite(unsigned char, GameState*);
bool sameSide(unsigned char, unsigned char, GameState*);
bool opposed(unsigned char, unsigned char, GameState*);
char getTeam(unsigned char, GameState*);

unsigned char uSet(unsigned char, char*, GameState*, unsigned char*);
unsigned char dSet(unsigned char, char*, GameState*, unsigned char*);
unsigned char lSet(unsigned char, char*, GameState*, unsigned char*);
unsigned char rSet(unsigned char, char*, GameState*, unsigned char*);
unsigned char ulSet(unsigned char, char*, GameState*, unsigned char*);
unsigned char urSet(unsigned char, char*, GameState*, unsigned char*);
unsigned char drSet(unsigned char, char*, GameState*, unsigned char*);
unsigned char dlSet(unsigned char, char*, GameState*, unsigned char*);

unsigned char u(unsigned char);
unsigned char d(unsigned char);
unsigned char l(unsigned char);
unsigned char r(unsigned char);
unsigned char ul(unsigned char);
unsigned char ur(unsigned char);
unsigned char dl(unsigned char);
unsigned char dr(unsigned char);
unsigned char row(unsigned char);
unsigned char col(unsigned char);

unsigned char getCol(unsigned char, unsigned char*);
unsigned char getRow(unsigned char, unsigned char*);
unsigned char getForwardSlash(unsigned char, unsigned char*);
unsigned char getBackSlash(unsigned char, unsigned char*);

/**************************************************************************************************
 Globals  */


/**************************************************************************************************
 Functions  */

void copyGameState(GameState* src, GameState* dst)
  {
    unsigned char i;

    dst->blackToMove = src->blackToMove;

    for(i = 0; i < _NONE; i++)
      dst->board[i] = src->board[i];

    return;
  }

/**************************************************************************************************
 Move generation and Application  */

void makeMove(Move* move, GameState* gs)
  {
    gs->board[move->to] = gs->board[move->from];
    gs->board[move->from] = _EMPTY;

    gs->blackToMove = !gs->blackToMove;                             //  Flip flag.

    return;
  }

/* Does not apply to real chess, but this is convenient for tree-search. */
void makeNullMove(GameState* gs)
  {
    gs->blackToMove = !gs->blackToMove;                             //  Flip flag.
    return;
  }

/* Return a character indicating who is to move now. */
char nowToMove(GameState* gs)
  {
    return (gs->blackToMove) ? 'b' : 'w';
  }

/* Return a character indicating who is next to move (once the side to move has played). */
char nextToMove(GameState* gs)
  {
    return (gs->blackToMove) ? 'w' : 'b';
  }

/* Return number of moves. Actual Move objects stored in given buffer. */
unsigned int getMoves(GameState* gs, Move* buffer)
  {
    unsigned int movesCtr = 0;
    Move potentialmoves[_MAX_MOVES];                                //  Assumes generous upper bound of moves per piece.
    unsigned int potentialmovesCtr = 0;
    unsigned int i;
    unsigned char index;

    for(index = 0; index < _NONE; index++)
      {
        if((gs->blackToMove && isBlack(index, gs)) || (!gs->blackToMove && isWhite(index, gs)))
          {
            potentialmovesCtr = getMovesIndex(index, gs, potentialmoves);
            if(potentialmovesCtr > 0)
              {
                for(i = 0; i < potentialmovesCtr; i++)
                  {
                    buffer[movesCtr].from = potentialmoves[i].from;
                    buffer[movesCtr].to = potentialmoves[i].to;
                    movesCtr++;
                  }
              }
          }
      }

    return movesCtr;
  }

/* Return number of moves. Actual Move objects stored in given buffer. */
unsigned int getMovesIndex(unsigned char index, GameState* gs, Move* buffer)
  {
    unsigned int movesCtr = 0;
    unsigned char target, intermediate;
    unsigned int i;
    unsigned char mileage;
    bool blocked;

    if(!isEmpty(index, gs))
      {
        mileage = columnMileage(index, gs);                         //  Column mileage.
        target = index;                                             //  Probe UP.
        for(i = 0; i < mileage; i++)
          target = u(target);
        if(target < _NONE && (opposed(index, target, board) || isEmpty(target, board)))
          {
            blocked = false;
            intermediate = index;
            for(i = 0; i < mileage - 1; i++)
              {
                intermediate = u(intermediate);
                if(opposed(index, intermediate, board))
                  blocked = true;
              }
            if(!blocked)
              {
                buffer[movesCtr].from = index;
                buffer[movesCtr].to = target;
                movesCtr++;
              }
          }

        target = index;                                             //  Probe DOWN.
        for(i = 0; i < mileage; i++)
          target = d(target);
        if(target < _NONE && (opposed(index, target, board) || isEmpty(target, board)))
          {
            blocked = false;
            intermediate = index;
            for(i = 0; i < mileage - 1; i++)
              {
                intermediate = d(intermediate);
                if(opposed(index, intermediate, board))
                  blocked = true;
              }
            if(!blocked)
              {
                buffer[movesCtr].from = index;
                buffer[movesCtr].to = target;
                movesCtr++;
              }
          }

        mileage = rowMileage(index, gs);                            //  Row mileage.
        target = index;                                             //  Probe LEFT.
        for(i = 0; i < mileage; i++)
          target = l(target);
        if(target < _NONE && (opposed(index, target, board) || isEmpty(target, board)))
          {
            blocked = false;
            intermediate = index;
            for(i = 0; i < mileage - 1; i++)
              {
                intermediate = l(intermediate);
                if(opposed(index, intermediate, board))
                  blocked = true;
              }
            if(!blocked)
              {
                buffer[movesCtr].from = index;
                buffer[movesCtr].to = target;
                movesCtr++;
              }
          }

        target = index;                                             //  Probe RIGHT.
        for(i = 0; i < mileage; i++)
          target = r(target);
        if(target < _NONE && (opposed(index, target, board) || isEmpty(target, board)))
          {
            blocked = false;
            intermediate = index;
            for(i = 0; i < mileage - 1; i++)
              {
                intermediate = r(intermediate);
                if(opposed(index, intermediate, board))
                  blocked = true;
              }
            if(!blocked)
              {
                buffer[movesCtr].from = index;
                buffer[movesCtr].to = target;
                movesCtr++;
              }
          }

        mileage = forwardSlashMileage(index, gs);                   //  Forward-slash mileage.
        target = index;                                             //  Probe UP-RIGHT.
        for(i = 0; i < mileage; i++)
          target = ur(target);
        if(target < _NONE && (opposed(index, target, board) || isEmpty(target, board)))
          {
            blocked = false;
            intermediate = index;
            for(i = 0; i < mileage - 1; i++)
              {
                intermediate = ur(intermediate);
                if(opposed(index, intermediate, board))
                  blocked = true;
              }
            if(!blocked)
              {
                buffer[movesCtr].from = index;
                buffer[movesCtr].to = target;
                movesCtr++;
              }
          }

        target = index;                                             //  Probe DOWN-LEFT.
        for(i = 0; i < mileage; i++)
          target = dl(target);
        if(target < _NONE && (opposed(index, target, board) || isEmpty(target, board)))
          {
            blocked = false;
            intermediate = index;
            for(i = 0; i < mileage - 1; i++)
              {
                intermediate = dl(intermediate);
                if(opposed(index, intermediate, board))
                  blocked = true;
              }
            if(!blocked)
              {
                buffer[movesCtr].from = index;
                buffer[movesCtr].to = target;
                movesCtr++;
              }
          }

        mileage = forwardSlashMileage(index, gs);                   //  Back-slash mileage.
        target = index;                                             //  Probe DOWN-RIGHT.
        for(i = 0; i < mileage; i++)
          target = dr(target);
        if(target < _NONE && (opposed(index, target, board) || isEmpty(target, board)))
          {
            blocked = false;
            intermediate = index;
            for(i = 0; i < mileage - 1; i++)
              {
                intermediate = dr(intermediate);
                if(opposed(index, intermediate, board))
                  blocked = true;
              }
            if(!blocked)
              {
                buffer[movesCtr].from = index;
                buffer[movesCtr].to = target;
                movesCtr++;
              }
          }

        target = index;                                             //  Probe UP-LEFT.
        for(i = 0; i < mileage; i++)
          target = ul(target);
        if(target < _NONE && (opposed(index, target, board) || isEmpty(target, board)))
          {
            blocked = false;
            intermediate = index;
            for(i = 0; i < mileage - 1; i++)
              {
                intermediate = ul(intermediate);
                if(opposed(index, intermediate, board))
                  blocked = true;
              }
            if(!blocked)
              {
                buffer[movesCtr].from = index;
                buffer[movesCtr].to = target;
                movesCtr++;
              }
          }
      }

    return movesCtr;
  }

/* Is the given move a capture on the given GameState? */
bool isCapture(Move* move, GameState* gs)
  {
    return !isEmpty(move->to, gs);
  }

/**************************************************************************************************
 Mileage  */

/* How many pieces in 'index's column? */
unsigned char columnMileage(unsigned char index, GameState* gs)
  {
    unsigned char ctr = 0;
    unsigned char i;
    unsigned char x[8];

    getCol(index, x);
    for(i = 0; i < 8; i++)
      {
        if(!isEmpty(x[i], gs))
          ctr++;
      }

    return ctr;
  }

/* How many pieces in 'index's row? */
unsigned char rowMileage(unsigned char index, GameState* gs)
  {
    unsigned char ctr = 0;
    unsigned char i;
    unsigned char x[8];

    getRow(index, x);
    for(i = 0; i < 8; i++)
      {
        if(!isEmpty(x[i], gs))
          ctr++;
      }

    return ctr;
  }

/* How many pieces in 'index's forward slash? */
unsigned char forwardSlashMileage(unsigned char index, GameState* gs)
  {
    unsigned char ctr = 0;
    unsigned char i;
    unsigned char x[8];                                             //  AT MOST 8 on an 8 x 8 board.
    unsigned char len;

    len = getForwardSlash(index, x);
    for(i = 0; i < len; i++)
      {
        if(!isEmpty(x[i], gs))
          ctr++;
      }

    return ctr;
  }

/* How many pieces in 'index's back slash? */
unsigned char backSlashMileage(unsigned char index, GameState* gs)
  {
    unsigned char ctr = 0;
    unsigned char i;
    unsigned char x[8];                                             //  AT MOST 8 on an 8 x 8 board.
    unsigned char len;

    len = getBackSlash(index, x);
    for(i = 0; i < len; i++)
      {
        if(!isEmpty(x[i], gs))
          ctr++;
      }

    return ctr;
  }

/**************************************************************************************************
 End-State Testing  */

/*  Given a board array, return
     GAME_ONGOING         if the state is not a win for either player
     GAME_OVER_BLACK_WINS if the state is a win for Black
     GAME_OVER_WHITE_WINS if the state is a win for White
     GAME_OVER_DRAW       if the state is a draw

  It is reported that Sid Sackson, LOA's creator, wanted to change the rules so that the player making a simultaneous connection wins.
  This could be implemented by consulting the side-to-move indicator in this function and then awarding the win to the OPPOSITE team
  because it HAS ALREADY CONNECTED.

  However, since most tournaments, including the Mind Sports Olympiad, recognize the first edition rules, we will, too. */
unsigned char isWin(GameState* gs)
  {
    unsigned char bIndex = 0;
    unsigned char wIndex = 0;
    unsigned char bBlock;
    unsigned char wBlock;

    while(bIndex < _NONE && !isBlack(bIndex, gs))                   //  Find first Black piece.
      bIndex++;

    while(wIndex < _NONE && !isWhite(wIndex, gs))                   //  Find first White piece.
      wIndex++;

    bBlock = bfs(bIndex, gs);                                       //  Are all black pieces connected?
    wBlock = bfs(wIndex, gs);                                       //  Are all white pieces connected?

    if(bBlock == totalBlack(gs) && wBlock < totalWhite(gs))
      return GAME_OVER_BLACK_WINS;
    if(wBlock == totalWhite(gs) && bBlock < totalBlack(gs))
      return GAME_OVER_WHITE_WINS;
    if(wBlock == totalWhite(gs) && bBlock == totalBlack(gs))        //  Simultaneous connection is a draw.
      return GAME_OVER_DRAW;

    return GAME_ONGOING;
  }

bool terminal(GameState* gs)
  {
    unsigned char win;

    win = isWin(gs);

    return (win != GAME_ONGOING);
  }

/* Starting at index, count up the number of pieces which can be reached by adjacency.
   (No need to fill a buffer.) */
unsigned char bfs(unsigned char index, GameState* gs)
  {
    unsigned char queue[_TOTAL_PIECES];                             //  The queue (of unsigned chars).
    unsigned char qlen = 0;                                         //  Queue length (increments of 1).

    unsigned char visited[_TOTAL_PIECES];                           //  Squares visited.
    unsigned char vlen = 0;
    bool notVisited;

    unsigned char nSq;                                              //  "Node" "popped" from "Queue".
    unsigned char i;

    queue[0] = index;                                               //  "Enqueue" the given index.
    qlen = 1;

    while(qlen > 0)
      {
        nSq = queue[0];                                             //  "Pop left".
        qlen--;                                                     //  Shorten the queue.

        for(i = 0; i < _TOTAL_PIECES - 1; i++)                      //  Shift everything down.
          queue[i] = queue[i + 1];

        notVisited = true;                                          //  Check whether already visited.
        if(vlen > 0)
          {
            i = 0;
            while(i < vlen && visited[i] != nSq)
              i++;

            if(i < vlen)
              notVisited = false;
          }

        if(notVisited)                                              //  "Node" has not been visited.
          {
            vlen++;
            visited[vlen - 1] = nSq;                                //  "Enqueue" this "node".

            if(u(nSq) < _NONE && sameSide(index, u(nSq), board))    //  "Enqueue" UP.
              {
                i = 0;
                while(i < qlen && queue[i] != u(nSq))               //  Do not enqueue duplicates.
                  i++;
                if(i == qlen)
                  {
                    qlen++;
                    queue[qlen - 1] = u(nSq);
                  }
              }
            if(ur(nSq) < _NONE && sameSide(index, ur(nSq), board))  //  "Enqueue" UP-RIGHT.
              {
                i = 0;
                while(i < qlen && queue[i] != ur(nSq))              //  Do not enqueue duplicates.
                  i++;
                if(i == qlen)
                  {
                    qlen++;
                    queue[qlen - 1] = ur(nSq);
                  }
              }
            if(r(nSq) < _NONE && sameSide(index, r(nSq), board))    //  "Enqueue" RIGHT.
              {
                i = 0;
                while(i < qlen && queue[i] != r(nSq))               //  Do not enqueue duplicates.
                  i++;
                if(i == qlen)
                  {
                    qlen++;
                    queue[qlen - 1] = r(nSq);
                  }
              }
            if(dr(nSq) < _NONE && sameSide(index, dr(nSq), board))  //  "Enqueue" DOWN-RIGHT.
              {
                i = 0;
                while(i < qlen && queue[i] != dr(nSq))              //  Do not enqueue duplicates.
                  i++;
                if(i == qlen)
                  {
                    qlen++;
                    queue[qlen - 1] = dr(nSq);
                  }
              }
            if(d(nSq) < _NONE && sameSide(index, d(nSq), board))    //  "Enqueue" DOWN.
              {
                i = 0;
                while(i < qlen && queue[i] != d(nSq))               //  Do not enqueue duplicates.
                  i++;
                if(i == qlen)
                  {
                    qlen++;
                    queue[qlen - 1] = d(nSq);
                  }
              }
            if(dl(nSq) < _NONE && sameSide(index, dl(nSq), board))  //  "Enqueue" DOWN-LEFT.
              {
                i = 0;
                while(i < qlen && queue[i] != dl(nSq))              //  Do not enqueue duplicates.
                  i++;
                if(i == qlen)
                  {
                    qlen++;
                    queue[qlen - 1] = dl(nSq);
                  }
              }
            if(l(nSq) < _NONE && sameSide(index, l(nSq), board))    //  "Enqueue" LEFT.
              {
                i = 0;
                while(i < qlen && queue[i] != l(nSq))               //  Do not enqueue duplicates.
                  i++;
                if(i == qlen)
                  {
                    qlen++;
                    queue[qlen - 1] = l(nSq);
                  }
              }
            if(ul(nSq) < _NONE && sameSide(index, ul(nSq), board))  //  "Enqueue" UP-LEFT.
              {
                i = 0;
                while(i < qlen && queue[i] != ul(nSq))              //  Do not enqueue duplicates.
                  i++;
                if(i == qlen)
                  {
                    qlen++;
                    queue[qlen - 1] = ul(nSq);
                  }
              }
          }
      }

    return vlen;
  }

/**************************************************************************************************
 Identities and Tests  */

/*  Is the given index i vacant? */
bool isEmpty(unsigned char i, GameState* gs)
  {
    return (gs->board[i] == _EMPTY);
  }

/*  Is the given index i occupied by a Black piece? */
bool isBlack(unsigned char i, GameState* gs)
  {
    return (gs->board[i] == _BLACK_PAWN);
  }

/*  Is the given index i occupied by a White piece? */
bool isWhite(unsigned char i, GameState* gs)
  {
    return (gs->board[i] == _WHITE_PAWN);
  }

/*  Is index i the same as index j
    in terms of both being White or both being Black or both being Empty? */
bool sameSide(unsigned char i, unsigned char j, GameState* gs)
  {
    return ((isWhite(i, gs) && isWhite(j, gs)) || (isBlack(i, gs) && isBlack(j, gs)));
  }

/*  More specific than same(), this function asks,
    "Are i and j on opposite teams?" */
bool opposed(unsigned char i, unsigned char j, GameState* gs)
  {
    return ((isWhite(i, gs) && isBlack(j, gs)) || (isBlack(i, gs) && isWhite(j, gs)));
  }

/* Return a character indicating which team 'index' belongs to. */
char getTeam(unsigned char index, GameState* gs)
  {
    if(isBlack(index, gs))
      return 'b';
    if(isWhite(index, gs))
      return 'w';
    return 'e';
  }

/**************************************************************************************************
 Board logic  */

/*  Return the index UP from the given i */
unsigned char u(unsigned char i)
  {
    if(i < _NONE)
      {
        if(row(i + 8) == row(i) + 1)
          return i + 8;
      }
    return _NONE;
  }

/*  Return the index DOWN from the given i */
unsigned char d(unsigned char i)
  {
    if(i < _NONE)
      {
        if(row(i - 8) == row(i) - 1 && row(i) != _NONE)
          return i - 8;
      }
    return _NONE;
  }

/*  Return the index LEFT from the given i */
unsigned char l(unsigned char i)
  {
    if(i < _NONE)
      {
        if(row(i - 1) == row(i))
          return i - 1;
      }
    return _NONE;
  }

/*  Return the index RIGHT from the given i */
unsigned char r(unsigned char i)
  {
    if(i < _NONE)
      {
        if(row(i + 1) == row(i))
          return i + 1;
      }
    return _NONE;
  }

/*  Return the index UP-LEFT from the given i */
unsigned char ul(unsigned char i)
  {
    if(i < _NONE)
      {
        if(row(i + 7) == row(i) + 1)
          return i + 7;
      }
    return _NONE;
  }

/*  Return the index UP-RIGHT from the given i */
unsigned char ur(unsigned char i)
  {
    if(i < _NONE)
      {
        if(row(i + 9) == row(i) + 1)
          return i + 9;
      }
    return _NONE;
  }

/*  Return the index DOWN-LEFT from the given i */
unsigned char dl(unsigned char i)
  {
    if(i < _NONE)
      {
        if(row(i - 9) == row(i) - 1 && row(i) != _NONE)
          return i - 9;
      }
    return _NONE;
  }

/*  Return the index DOWN-RIGHT from the given i */
unsigned char dr(unsigned char i)
  {
    if(i < _NONE)
      {
        if(row(i - 7) == row(i) - 1 && row(i) != _NONE)
          return i - 7;
      }
    return _NONE;
  }

/*  Compute the COLUMN in which given index is included */
unsigned char col(unsigned char i)
  {
    if(i < _NONE)
      return i & 7;                                                 //  i & 7 == i % 8 because 8 is a power of 2
    return _NONE;
  }

/*  Compute the ROW in which given index is included */
unsigned char row(unsigned char i)
  {
    if(i < _NONE)
      return (i - (i & 7)) / 8;                                     //  i & 7 == i % 8 because 8 is a power of 2
    return _NONE;
  }

/* Write to given buffer all indices in the column of the given index. */
unsigned char getCol(unsigned char index, unsigned char* c)
  {
    unsigned char i, j, k = 0;

    if(index < _NONE)
      {
        i = col(index);
        j = i + 57;

        while(i < j)
          {
            c[k] = i;
            i += 8;
            k++;
          }
      }
    else
      {
        for(i = 0; i < 8; i++)
          c[i] = _NONE;
      }

    return 8;
  }

/* Write to given buffer all indices in the row of the given index. */
unsigned char getRow(unsigned char index, unsigned char* c)
  {
    unsigned char i, j, k = 0;

    if(index < _NONE)
      {
        i = row(index) * 8;
        j = i + 8;

        while(i < j)
          {
            c[k] = i;
            i++;
            k++;
          }
      }
    else
      {
        for(i = 0; i < 8; i++)
          c[i] = _NONE;
      }

    return 8;
  }

/* Write to given buffer all indices in the forward slash of the given index. */
unsigned char getForwardSlash(unsigned char index, unsigned char* c)
  {
    unsigned char pos;
    unsigned char len = 0;

    pos = index;                                                    //  "Rewind" dl() as far as we can.
    while(dl(pos) < _NONE)
      pos = dl(pos);

    do                                                              //  Now "forward" ur() as far as we can.
      {
        c[len++] = stop;
        pos = ur(pos);
      } while(ur(pos) < _NONE);

    return len;
  }

/* Write to given buffer all indices in the back slash of the given index. */
unsigned char getBackSlash(unsigned char index, unsigned char* c)
  {
    unsigned char pos;
    unsigned char len = 0;

    pos = index;                                                    //  "Rewind" dr() as far as we can.
    while(dr(pos) < _NONE)
      pos = dr(pos);

    do                                                              //  Now "forward" ul() as far as we can.
      {
        c[len++] = stop;
        pos = ul(pos);
      } while(ul(pos) < _NONE);

    return len;
  }

#endif