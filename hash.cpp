/*

 ./hash 126 0 0 0 0 0 0 126 0 129 129 129 129 129 129 0 128

*/

#include <stdio.h>
#include <stdlib.h>

#include "zobrist.h"                                                /* Include the Zobrist hasher, which is an array of unsigned long longs (64-bit ints). */

#define _GAMESTATE_BYTE_SIZE                    17                  /* Number of bytes needed to encode a game state. */
#define _NONE                                   64                  /* Required as a limit without #include "gamestate.h". */

/**************************************************************************************************
 Prototypes  */

unsigned long long hash(unsigned char*);

/**************************************************************************************************
 Globals  */

unsigned long long zobristHashTable[ZHASH_TABLE_SIZE] = {};

/**************************************************************************************************
 Main  */

int main(int argc, char* argv[])
  {
    unsigned char inputByteArray[_GAMESTATE_BYTE_SIZE];
    unsigned long long h;
    unsigned char i;

    for(i = 0; i < _GAMESTATE_BYTE_SIZE; i++)
      inputByteArray[i] = (unsigned char)atoi(argv[1 + i]);

    h = hash(inputByteArray);
    printf("%llu", h);

    return 0;
  }

/**************************************************************************************************
 Zobrist hashing  */

/* Hash the given byte array "hashInputBuffer". */
unsigned long long hash(unsigned char* hashInputBuffer)
  {
    unsigned long long h = 0L;
    unsigned int index;
    unsigned int i;
    unsigned long long ull8;                                        //  The unsigned long long we will actually use to hash.

    if((hashInputBuffer[0] & 128) == 128)                           //  Hash the side to move.
      {
        ull8 = zobristHashTable[W_TO_MOVE];
        h ^= ull8;
      }
    if((hashInputBuffer[0] & 64) == 64)                             //  Hash white's castling data.
      {
        ull8 = zobristHashTable[W_KINGSIDE_CASTLE];
        h ^= ull8;
      }
    if((hashInputBuffer[0] & 32) == 32)
      {
        ull8 = zobristHashTable[W_QUEENSIDE_CASTLE];
        h ^= ull8;
      }
    if((hashInputBuffer[0] & 16) == 16)
      {
        ull8 = zobristHashTable[W_CASTLED];
        h ^= ull8;
      }
    if((hashInputBuffer[0] & 8) == 8)                               //  Hash black's castling data.
      {
        ull8 = zobristHashTable[B_KINGSIDE_CASTLE];
        h ^= ull8;
      }
    if((hashInputBuffer[0] & 4) == 4)
      {
        ull8 = zobristHashTable[B_QUEENSIDE_CASTLE];
        h ^= ull8;
      }
    if((hashInputBuffer[0] & 2) == 2)
      {
        ull8 = zobristHashTable[B_CASTLED];
        h ^= ull8;
      }

    if((hashInputBuffer[1] & 128) == 128)                           //  Hash whether a pawn's doulbe move previously occurred in column A.
      {
        ull8 = zobristHashTable[PREV_DOUBLE_COL_A];
        h ^= ull8;
      }
    else if((hashInputBuffer[1] & 64) == 64)                        //  Hash whether a pawn's doulbe move previously occurred in column B.
      {
        ull8 = zobristHashTable[PREV_DOUBLE_COL_B];
        h ^= ull8;
      }
    else if((hashInputBuffer[1] & 32) == 32)                        //  Hash whether a pawn's doulbe move previously occurred in column C.
      {
        ull8 = zobristHashTable[PREV_DOUBLE_COL_C];
        h ^= ull8;
      }
    else if((hashInputBuffer[1] & 16) == 16)                        //  Hash whether a pawn's doulbe move previously occurred in column D.
      {
        ull8 = zobristHashTable[PREV_DOUBLE_COL_D];
        h ^= ull8;
      }
    else if((hashInputBuffer[1] & 8) == 8)                          //  Hash whether a pawn's doulbe move previously occurred in column E.
      {
        ull8 = zobristHashTable[PREV_DOUBLE_COL_E];
        h ^= ull8;
      }
    else if((hashInputBuffer[1] & 4) == 4)                          //  Hash whether a pawn's doulbe move previously occurred in column F.
      {
        ull8 = zobristHashTable[PREV_DOUBLE_COL_F];
        h ^= ull8;
      }
    else if((hashInputBuffer[1] & 2) == 2)                          //  Hash whether a pawn's doulbe move previously occurred in column G.
      {
        ull8 = zobristHashTable[PREV_DOUBLE_COL_G];
        h ^= ull8;
      }
    else if((hashInputBuffer[1] & 1) == 1)                          //  Hash whether a pawn's doulbe move previously occurred in column H.
      {
        ull8 = zobristHashTable[PREV_DOUBLE_COL_H];
        h ^= ull8;
      }

    i = 2;
    for(index = 0; index < _NONE; index++)
      {
                                                                    //  There can be no pawns on row 1 or row 8.
        if(hashInputBuffer[i] == _WHITE_PAWN && index >= 8 && index < 56)
          {
            ull8 = zobristHashTable[(WP_A2 + index - 8)];
            h ^= ull8;
          }
        else if(hashInputBuffer[i] == _WHITE_KNIGHT)
          {
            ull8 = zobristHashTable[(WN_A1 + index)];
            h ^= ull8;
          }
        else if(hashInputBuffer[i] == _WHITE_BISHOP)
          {
            ull8 = zobristHashTable[(WB_A1 + index)];
            h ^= ull8;
          }
        else if(hashInputBuffer[i] == _WHITE_ROOK)
          {
            ull8 = zobristHashTable[(WR_A1 + index)];
            h ^= ull8;
          }
        else if(hashInputBuffer[i] == _WHITE_QUEEN)
          {
            ull8 = zobristHashTable[(WQ_A1 + index)];
            h ^= ull8;
          }
        else if(hashInputBuffer[i] == _WHITE_KING)
          {
            ull8 = zobristHashTable[(WK_A1 + index)];
            h ^= ull8;
          }
                                                                    //  There can be no pawns on row 1 or row 8.
        else if(hashInputBuffer[i] == _BLACK_PAWN && index >= 8 && index < 56)
          {
            ull8 = zobristHashTable[(BP_A2 + index - 8)];
            h ^= ull8;
          }
        else if(hashInputBuffer[i] == _BLACK_KNIGHT)
          {
            ull8 = zobristHashTable[(BN_A1 + index)];
            h ^= ull8;
          }
        else if(hashInputBuffer[i] == _BLACK_BISHOP)
          {
            ull8 = zobristHashTable[(BB_A1 + index)];
            h ^= ull8;
          }
        else if(hashInputBuffer[i] == _BLACK_ROOK)
          {
            ull8 = zobristHashTable[(BR_A1 + index)];
            h ^= ull8;
          }
        else if(hashInputBuffer[i] == _BLACK_QUEEN)
          {
            ull8 = zobristHashTable[(BQ_A1 + index)];
            h ^= ull8;
          }
        else if(hashInputBuffer[i] == _BLACK_KING)
          {
            ull8 = zobristHashTable[(BK_A1 + index)];
            h ^= ull8;
          }

        i++;
      }

    return h;
  }
