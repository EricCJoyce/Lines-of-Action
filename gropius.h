#ifndef __GROPIUS_H
#define __GROPIUS_H

#include "gamestate.h"
                                                                    //  Weights determined by TDLeaf(lambda).
#define WEIGHT_CONCENTRATION              1.0
#define WEIGHT_CENTRALIZATION             1.0
#define WEIGHT_CENTER_OF_MASS             1.0
#define WEIGHT_QUADS                      1.0
#define WEIGHT_MOBILITY                   1.0
#define WEIGHT_WALLS_COM                  1.0
#define WEIGHT_WALLS_INNER4               1.0
#define WEIGHT_WALLS_INNER12              1.0
#define WEIGHT_CONNECTEDNESS              6.0
#define WEIGHT_UNIFORMITY                 1.0

#define CENTRALIZATION_WEIGHT_0          -8.0                       /* Penalty per piece occupying the corners */
#define CENTRALIZATION_WEIGHT_1          -2.5                       /* Penalty per piece occupying the edges */
#define CENTRALIZATION_WEIGHT_2          -2.0                       /* Penalty per piece occupying the outer orbit */
#define CENTRALIZATION_WEIGHT_3           1.0                       /* Reward per piece occupying the inner orbit */
#define CENTRALIZATION_WEIGHT_4           2.5                       /* Reward per piece occupying the center 12 */
#define CENTRALIZATION_WEIGHT_5           5.0                       /* Reward per piece occupying the center 4 */

#define QUAD3_BONUS                       1.0                       /* No bonus for Quad-1s or Quad-2s. */
#define QUAD4_BONUS                       2.0

#define QUAD_QUALIFYING_DISTANCE_TO_COM   2

#define MOBILITY_MOVE_BONUS               2
#define MOBILITY_EDGE_MOVE_PENALTY       -1

/**************************************************************************************************
 Typedefs  */


/**************************************************************************************************
 Prototypes  */

unsigned int getMovesForTeam(bool, GameState*, Move*);
float score(GameState*);

float concentration(unsigned char*, unsigned char);
unsigned char centerOfMass(unsigned char*, unsigned char);
unsigned char orbit(unsigned char);
unsigned char zones(unsigned char**);
float evaluateCenterOfMass(unsigned char);
float centralization(unsigned char*, unsigned char);
float quads(unsigned char*, unsigned char, unsigned char, GameState*);
bool quadDistance(unsigned char*, unsigned char, unsigned char, unsigned char);
float mobility(Move*, unsigned char, unsigned char, GameState*);
unsigned char wallsCOM(unsigned char*, unsigned char, unsigned char, GameState*);
unsigned char wallsCenter4(unsigned char*, unsigned char, GameState*);
unsigned char wallsCenter12(unsigned char*, unsigned char, GameState*);
unsigned char walls(unsigned char*, unsigned char, unsigned char, GameState*);
unsigned char wallsList(unsigned char*, unsigned char, unsigned char*, unsigned char, GameState*);
float connectedness(unsigned char*, unsigned char, GameState*);
float uniformity(unsigned char*, unsigned char);

/**************************************************************************************************
 Team Moves  */

/* Differs from gamestate.h getMoves() because you may specify a team not necessarily now to move. */
unsigned int getMovesForTeam(bool black, GameState* gs, Move* buffer)
  {
    unsigned int movesCtr = 0;
    Move potentialmoves[_MAX_MOVES];                                //  Assumes generous upper bound of moves per piece.
    unsigned int potentialmovesCtr = 0;
    unsigned int i;
    unsigned char index;

    for(index = 0; index < _NONE; index++)
      {
        if((black && isBlack(index, gs)) || (!black && isWhite(index, gs)))
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

/**************************************************************************************************
 Evaluation  */

/* Negamax rule: ALWAYS EVALUATE FOR THE SIDE TO MOVE. */
float score(GameState* gs)
  {
    float h = 0.0;
    unsigned char win;

    unsigned char blackMaterial[_TOTAL_PIECES];                     //  Indices of all Black material.
    unsigned char blackMaterialLength = 0;
    unsigned char whiteMaterial[_TOTAL_PIECES];                     //  Indices of all White material.
    unsigned char whiteMaterialLength = 0;

    Move blackMoves[_MAX_MOVES];                                    //  Arrays of Move structs. Generous upper bound.
    unsigned char blackMovesLength = 0;
    Move whiteMoves[_MAX_MOVES];
    unsigned char whiteMovesLength = 0;

    unsigned char blackCoM;                                         //  Index of Black's Center of Mass.
    unsigned char whiteCoM;                                         //  Index of White's Center of Mass.

    win = isWin(gs);
    if(win != GAME_ONGOING)
      {
        if(win == GAME_OVER_BLACK_WINS && gs->blackToMove)          //  Discourage white from GIVING black the win.
          return INFINITY;
        else if(win == GAME_OVER_WHITE_WINS && !gs->blackToMove)    //  Discourage black from GIVING white the win.
          return INFINITY;
        if(win == GAME_OVER_DRAW)
          return 0.0;
        else                                                        //  e.g. It is black's turn but white has won; it is white's tunr but black has won.
          return -INFINITY;
      }

    //////////////////////////////////////////////////////////////////  Compute the following only ONCE.
    blackMaterialLength = getBlack(gs, blackMaterial);              //  unsigned chars
    whiteMaterialLength = getWhite(gs, whiteMaterial);              //  unsigned chars

    blackMovesLength = getMovesForTeam(true, gs, blackMoves);       //  Moves
    whiteMovesLength = getMovesForTeam(false, gs, whiteMoves);      //  Moves

    blackCoM = centerOfMass(blackMaterial, blackMaterialLength);    //  Index
    whiteCoM = centerOfMass(whiteMaterial, whiteMaterialLength);    //  Index

    if(gs->blackToMove)  /////////////////////////////////////////////  BLACK
      {
        h += (concentration(blackMaterial, blackMaterialLength) - concentration(whiteMaterial, whiteMaterialLength)) * WEIGHT_CONCENTRATION;
        h += (centralization(blackMaterial, blackMaterialLength) - centralization(whiteMaterial, whiteMaterialLength)) * WEIGHT_CENTRALIZATION;
        h += (evaluateCenterOfMass(blackCoM) - evaluateCenterOfMass(whiteCoM)) * WEIGHT_CENTER_OF_MASS;
        h += (quads(blackMaterial, blackMaterialLength, blackCoM, gs) - quads(whiteMaterial, whiteMaterialLength, whiteCoM, gs)) * WEIGHT_QUADS;
        h += (mobility(blackMoves, blackMovesLength, blackMaterialLength, gs) - mobility(whiteMoves, whiteMovesLength, whiteMaterialLength, gs)) * WEIGHT_MOBILITY;
        h += (wallsCOM(whiteMaterial, whiteMaterialLength, whiteCoM, gs) - wallsCOM(blackMaterial, blackMaterialLength, blackCoM, gs)) * WEIGHT_WALLS_COM;
        h += (wallsCenter4(whiteMaterial, whiteMaterialLength, gs) - wallsCenter4(blackMaterial, blackMaterialLength, gs)) * WEIGHT_WALLS_INNER4;
        h += (wallsCenter12(whiteMaterial, whiteMaterialLength, gs) - wallsCenter12(blackMaterial, blackMaterialLength, gs)) * WEIGHT_WALLS_INNER12;
        h += (connectedness(blackMaterial, blackMaterialLength, gs) - connectedness(whiteMaterial, whiteMaterialLength, gs)) * WEIGHT_CONNECTEDNESS;
        h += (uniformity(blackMaterial, blackMaterialLength) - uniformity(whiteMaterial, whiteMaterialLength)) * WEIGHT_UNIFORMITY;
      }
    else  ////////////////////////////////////////////////////////////  WHITE
      {
        h += (concentration(whiteMaterial, whiteMaterialLength) - concentration(blackMaterial, blackMaterialLength)) * WEIGHT_CONCENTRATION;
        h += (centralization(whiteMaterial, whiteMaterialLength) - centralization(blackMaterial, blackMaterialLength)) * WEIGHT_CENTRALIZATION;
        h += (evaluateCenterOfMass(whiteCoM) - evaluateCenterOfMass(blackCoM)) * WEIGHT_CENTER_OF_MASS;
        h += (quads(whiteMaterial, whiteMaterialLength, whiteCoM, gs) - quads(blackMaterial, blackMaterialLength, blackCoM, gs)) * WEIGHT_QUADS;
        h += (mobility(whiteMoves, whiteMovesLength, whiteMaterialLength, gs) - mobility(blackMoves, blackMovesLength, blackMaterialLength, gs)) * WEIGHT_MOBILITY;
        h += (wallsCOM(blackMaterial, blackMaterialLength, blackCoM, gs) - wallsCOM(whiteMaterial, whiteMaterialLength, whiteCoM, gs)) * WEIGHT_WALLS_COM;
        h += (wallsCenter4(blackMaterial, blackMaterialLength, gs) - wallsCenter4(whiteMaterial, whiteMaterialLength, gs)) * WEIGHT_WALLS_INNER4;
        h += (wallsCenter12(blackMaterial, blackMaterialLength, gs) - wallsCenter12(whiteMaterial, whiteMaterialLength, gs)) * WEIGHT_WALLS_INNER12;
        h += (connectedness(whiteMaterial, whiteMaterialLength, gs) - connectedness(blackMaterial, blackMaterialLength, gs)) * WEIGHT_CONNECTEDNESS;
        h += (uniformity(whiteMaterial, whiteMaterialLength) - uniformity(blackMaterial, blackMaterialLength)) * WEIGHT_UNIFORMITY;
      }

    return h;
  }

/**************************************************************************************************
 Concentration
 How close pieces are to each other.
 . . . . . . . .     . . . . . . . .     . . . . . . . .     . . . . . . . .     . . . . . . . .
 . . . . . . . .     . . . . . . . .     . . . . . . . .     . . . . . . . .     . . . . . . . .
 . . . . . . . B     . . . . . . B B     . . . . . . B B     . . . . . . B B     . . . . . . . B
 . . . . . . . .     . . . . . . . .     . . . . . . . .     . . . . . . . .     . . . . . . . .
 . . . . . . . .     . . . . . . . .     . . . . . . . .     . . . . . . . .     . . . . B . . .
 . . . . . . . .     . . . . . . . .     . . . . . . . .     . . B . . . . .     . . . . . . . .
 . . . . . . . .     . . . . . . . .     . B . . . . . .     . B . . . . . .     . . . . . . . .
 B . . . . . . .     B . . . . . . .     B . . . . . . .     B . . . . . . .     B . . . . . . .
 conc.               conc.               conc.               conc.               conc.
   = 0.142857143       = 0.777777778       = 1.338095238       = 1.839365079       = 0.484126984
   */
float concentration(unsigned char* team, unsigned char len)
  {
    float h = 0.0;
    unsigned char i, j;

    for(i = 0; i < len; i++)                                        //  For each piece, compute distance to all other friendly pieces.
      {
        for(j = 0; j < len; j++)
          {
            if(i != j)                                              //  Exclude comparisons to self.
              h += 1.0 / (float)distance(team[i], team[j]);         //  Accumulate values in range [0.142857143, 1.0]. Higher is better.
          }
      }

    h /= (float)len;                                                //  Divide by the number of pieces.

    return h;
  }

/**************************************************************************************************
 Center of Mass
 Determine the index of a team's center of mass  */
unsigned char centerOfMass(unsigned char* team, unsigned char teamLen)
  {
    unsigned char sumx = 0;
    unsigned char sumy = 0;
    unsigned char i;

    for(i = 0; i < teamLen; i++)
      {
        sumx += col(team[i]);
        sumy += row(team[i]);
      }

    return (unsigned char)((int)(round((float)sumy / (float)teamLen)) * 8 + (int)(round((float)sumx / (float)teamLen)));
  }

/* Returns an integer in range [0, 5] reflecting the centrality of 'index' */
unsigned char orbit(unsigned char index)
  {
    unsigned char map[_NONE];
    unsigned char lookup;

    zones(map);
    lookup = map[index];

    return lookup;
  }

/*    0 1 2 2 2 2 1 0
      1 3 3 3 3 3 3 1
      2 3 4 4 4 4 3 2
      2 3 4 5 5 4 3 2
      2 3 4 5 5 4 3 2
      2 3 4 4 4 4 3 2
      1 3 3 3 3 3 3 1
      0 1 2 2 2 2 1 0
  Return this map.  */
unsigned char zones(unsigned char* map)
  {
    map[0]  = 0; map[1]  = 1; map[2]  = 2; map[3]  = 2; map[4]  = 2; map[5]  = 2; map[6]  = 1; map[7]  = 0;
    map[8]  = 1; map[9]  = 3; map[10] = 3; map[11] = 3; map[12] = 3; map[13] = 3; map[14] = 3; map[15] = 1;
    map[16] = 2; map[17] = 3; map[18] = 4; map[19] = 4; map[20] = 4; map[21] = 4; map[22] = 3; map[23] = 2;
    map[24] = 2; map[25] = 3; map[26] = 4; map[27] = 5; map[28] = 5; map[29] = 4; map[30] = 3; map[31] = 2;
    map[32] = 2; map[33] = 3; map[34] = 4; map[35] = 5; map[36] = 5; map[37] = 4; map[38] = 3; map[39] = 2;
    map[40] = 2; map[41] = 3; map[42] = 4; map[43] = 4; map[44] = 4; map[45] = 4; map[46] = 3; map[47] = 2;
    map[48] = 1; map[49] = 3; map[50] = 3; map[51] = 3; map[52] = 3; map[53] = 3; map[54] = 3; map[55] = 1;
    map[56] = 0; map[57] = 1; map[58] = 2; map[59] = 2; map[60] = 2; map[61] = 2; map[62] = 1; map[63] = 0;

    return _NONE;
  }

/* Look up the given center of mass in the "orbit" map */
float evaluateCenterOfMass(unsigned char CoM)
  {
    float h = 0.0;
    signed char weights[6];

    weights[0] = CENTRALIZATION_WEIGHT_0;                           //  From farthest to most central: 0 - 5
    weights[1] = CENTRALIZATION_WEIGHT_1;
    weights[2] = CENTRALIZATION_WEIGHT_2;
    weights[3] = CENTRALIZATION_WEIGHT_3;
    weights[4] = CENTRALIZATION_WEIGHT_4;
    weights[5] = CENTRALIZATION_WEIGHT_5;

    h += (float)weights[ orbit(CoM) ];

    return h;
  }

/**************************************************************************************************
 Centralization
 Control of the center  */
float centralization(unsigned char* team, unsigned char len)
  {
    float h = 0.0;
    unsigned char map[_NONE];                                       //  Store the map created by zones().
    unsigned char orbitCtrs[6];
    float weights[6];
    unsigned char i;

    zones(map);

    for(i = 0; i < 6; i++)
      orbitCtrs[i] = 0;

    weights[0] = CENTRALIZATION_WEIGHT_0;                           //  From farthest to most central: 0 - 5.
    weights[1] = CENTRALIZATION_WEIGHT_1;
    weights[2] = CENTRALIZATION_WEIGHT_2;
    weights[3] = CENTRALIZATION_WEIGHT_3;
    weights[4] = CENTRALIZATION_WEIGHT_4;
    weights[5] = CENTRALIZATION_WEIGHT_5;

    for(i = 0; i < len; i++)                                        //  Count occupancies in each orbit.
      orbitCtrs[map[team[i]]]++;

    for(i = 0; i < 6; i++)                                          //  Score occupancies per orbit.
      h += (float)orbitCtrs[i] * weights[i];

    return h;
  }

/**************************************************************************************************
 Quads
 Quad 3's and 4's are good because they are hard to break apart or circumvent.
 Reward quads double if they are near the center of mass.
 The array 'team' contains the indices of Black or White, returned either by getBlack() or getWhite(). */
float quads(unsigned char* team, unsigned char len, unsigned char CoM, GameState* gs)
  {
    float h = 0.0;
    unsigned char quadBuffer[4];                                    //  Contains the valid (not off the board) indices of
                                                                    //    a 4-quad, 3-quad, 2-quad, or 1-quad.
    unsigned char i, j;                                             //  Iterators
    unsigned char index;
    bool inRange;
    unsigned char teamCtr;                                          //  Count quad occupancy.
    char side = getTeam(team[0], gs);                               //  Which team are we counting up?
                                                                    //  (The array 'team' MUST have at least 1 index in it.)
    for(i = 0; i < 7; i++)
      {
        for(j = 0; j < 7; j++)
          {
            teamCtr = 0;

            index = i * 8 + j;
            if(getTeam(index, gs) == side)                          //  Check lower-left of 2 x 2 window.
              {
                quadBuffer[teamCtr] = index;
                teamCtr++;
              }

            index = i * 8 + j + 1;
            if(getTeam(index, gs) == side)                          //  Check lower-right of 2 x 2 window.
              {
                quadBuffer[teamCtr] = index;
                teamCtr++;
              }

            index = (i + 1) * 8 + j;
            if(getTeam(index, gs) == side)                          //  Check upper-left of 2 x 2 window.
              {
                quadBuffer[teamCtr] = index;
                teamCtr++;
              }

            index = (i + 1) * 8 + j + 1;
            if(getTeam(index, gs) == side)                          //  Check upper-right of 2 x 2 window.
              {
                quadBuffer[teamCtr] = index;
                teamCtr++;
              }

            inRange = quadDistance(quadBuffer, teamCtr, CoM, QUAD_QUALIFYING_DISTANCE_TO_COM);

            if(teamCtr == 4)
              {
                h += QUAD4_BONUS;
                if(inRange)
                  h += QUAD4_BONUS;
              }
            else if(teamCtr == 3)
              {
                h += QUAD3_BONUS;
                if(inRange)
                  h += QUAD3_BONUS;
              }
          }
      }

    return h;
  }

/* Returns true if any part of the given quad is within 'distanceCutoff' squares of the given Center of Mass */
bool quadDistance(unsigned char* quad, unsigned char quadLen, unsigned char CoM, unsigned char distanceCutoff)
  {
    bool h = false;
    unsigned char q;

    for(q = 0; q < quadLen; q++)
      {
        if(distance(quad[q], CoM) <= distanceCutoff)
          {
            h = true;
            break;
          }
      }
    return h;
  }

/**************************************************************************************************
 Mobility
 Rewards having more moves available per piece  */
float mobility(Move* posMoves, unsigned char posMovesLen, unsigned char posLen, GameState* gs)
  {
    unsigned char i;
    float h = 0.0;

    for(i = 0; i < posMovesLen; i++)
      {
        h += (float)MOBILITY_MOVE_BONUS;                            //  Award for having a move.

        if( col(posMoves[i].to) == 0 || col(posMoves[i].to) == 7 ||
            row(posMoves[i].to) == 0 || row(posMoves[i].to) == 7 )
          h += (float)MOBILITY_EDGE_MOVE_PENALTY;                   //  Diminished slightly for being on the edge.

        if(!isEmpty(posMoves[i].to, gs))
          h += (float)MOBILITY_MOVE_BONUS;                          //  Award for having an attack.
      }

    return h / (float)posLen;                                       //  Per piece.
  }

/**************************************************************************************************
 Walls  */

/* Walls to center of mass
   MIA rewards a block on the opponent's reaching the center...
   I will block the opponent's reaching its center of mass  */
unsigned char wallsCOM(unsigned char* negTeam, unsigned char negTeamLen, unsigned char negCoM, GameState* gs)
  {
    return walls(negTeam, negTeamLen, negCoM, gs);
  }

/* Walls to center of mass
   Walls to board center (central 4 squares)  */
unsigned char wallsCenter4(unsigned char* negTeam, unsigned char negTeamLen, GameStae* gs)
  {
    unsigned char h = 0;
    unsigned char dstList[4];

    dstList[0]  = 27;                                               //  . . . . . . . .
    dstList[1]  = 28;                                               //  . . . . . . . .
    dstList[2]  = 35;                                               //  . . . . . . . .
    dstList[3]  = 36;                                               //  . . . X X . . .
                                                                    //  . . . X X . . .
    h += wallsList(negTeam, negTeamLen, dstList, 4, gs);            //  . . . . . . . .
                                                                    //  . . . . . . . .
                                                                    //  . . . . . . . .
    return h;
  }

/* Walls to center of mass
   Walls to board center (central 12 squares)  */
unsigned char wallsCenter12(unsigned char* negTeam, unsigned char negTeamLen, GameState* gs)
  {
    unsigned char h = 0;
    unsigned char dstList[12];

    dstList[0]  = 18;                                               //  . . . . . . . .
    dstList[1]  = 19;                                               //  . . . . . . . .
    dstList[2]  = 20;                                               //  . . X X X X . .
    dstList[3]  = 21;                                               //  . . X . . X . .
    dstList[4]  = 26;                                               //  . . X . . X . .
    dstList[5]  = 29;                                               //  . . X X X X . .
    dstList[6]  = 34;                                               //  . . . . . . . .
    dstList[7]  = 37;                                               //  . . . . . . . .
    dstList[8]  = 42;
    dstList[9]  = 43;
    dstList[10] = 44;
    dstList[11] = 45;

    h += wallsList(negTeam, negTeamLen, dstList, 12, gs);

    return h;
  }

/* This function works by looking at where negTeam COULD be going and then adding to a running score
   for every instance where that trip is made impossible because of the opposing team's presence.
   . . . . . . . .
   . . . . . . . .
   . . . . . . . .
   . . . . . . . .
   . ------+ . . .  <-- Suppose this is the destination
   . | / . . . . .
   . B W . . . . .  <-- W walls B in horizontal travel
   . . . . . . . .

   */
unsigned char walls(unsigned char* negTeam, unsigned char negTeamLen, unsigned char dst, GameState* gs)
  {
    unsigned char h = 0;
    unsigned char mileage;
    unsigned char i, j;
    unsigned char target;
    unsigned char next;
    bool blocked;

    for(i = 0; i < negTeamLen; i++)
      {
        mileage = columnMileage(negTeam[i], gs);    //////////////////  Column mileage
        target = negTeam[i];                        //////////////////  Find UP-path
        for(j = 0; j < mileage; j++)                                //  to a presumed destination named 'target'
          target = u(target);
        j = 0;                                                      //  Is that path blocked?
        blocked = false;
        next = u(negTeam[i]);
        while(j < mileage && !blocked && next < _NONE)
          {
            if(opposed(negTeam[i], next, gs))
              blocked = true;
            next = u(next);
            j++;
          }
                                                                    //  Is the blocked presumed destination
                                                                    //  closer to the true (query) destination?
        if(blocked && distance(negTeam[i], dst) > distance(target, dst))
          h++;

        target = negTeam[i];                        //////////////////  Find DOWN-path
        for(j = 0; j < mileage; j++)                                //  to a presumed destination named 'target'
          target = d(target);
        j = 0;                                                      //  Is that path blocked?
        blocked = false;
        next = d(negTeam[i]);
        while(j < mileage && !blocked && next < _NONE)
          {
            if(opposed(negTeam[i], next, gs))
              blocked = true;
            next = d(next);
            j++;
          }
                                                                    //  Is the blocked presumed destination
                                                                    //  closer to the true (query) destination?
        if(blocked && distance(negTeam[i], dst) > distance(target, dst))
          h++;

        //////////////////////////////////////////////////////////////
        mileage = rowMileage(negTeam[i], gs);    /////////////////////  Row mileage
        target = negTeam[i];                     /////////////////////  Find LEFT-path
        for(j = 0; j < mileage; j++)                                //  to a presumed destination named 'target'
          target = l(target);
        j = 0;                                                      //  Is that path blocked?
        blocked = false;
        next = l(negTeam[i]);
        while(j < mileage && !blocked && next < _NONE)
          {
            if(opposed(negTeam[i], next, gs))
              blocked = true;
            next = l(next);
            j++;
          }
                                                                    //  Is the blocked presumed destination
                                                                    //  closer to the true (query) destination?
        if(blocked && distance(negTeam[i], dst) > distance(target, dst))
          h++;

        target = negTeam[i];                     /////////////////////  Find RIGHT-path
        for(j = 0; j < mileage; j++)                                //  to a presumed destination named 'target'
          target = r(target);
        j = 0;                                                      //  Is that path blocked?
        blocked = false;
        next = r(negTeam[i]);
        while(j < mileage && !blocked && next < _NONE)
          {
            if(opposed(negTeam[i], next, gs))
              blocked = true;
            next = r(next);
            j++;
          }
                                                                    //  Is the blocked presumed destination
                                                                    //  closer to the true (query) destination?
        if(blocked && distance(negTeam[i], dst) > distance(target, dst))
          h++;

        //////////////////////////////////////////////////////////////
        mileage = forwardSlashMileage(negTeam[i], gs);    ////////////  Forward-slash mileage
        target = negTeam[i];                              ////////////  Find UP-RIGHT-path
        for(j = 0; j < mileage; j++)                                //  to a presumed destination named 'target'
          target = ur(target);
        j = 0;                                                      //  Is that path blocked?
        blocked = false;
        next = ur(negTeam[i]);
        while(j < mileage && !blocked && next < _NONE)
          {
            if(opposed(negTeam[i], next, gs))
              blocked = true;
            next = ur(next);
            j++;
          }
                                                                    //  Is the blocked presumed destination
                                                                    //  closer to the true (query) destination?
        if(blocked && distance(negTeam[i], dst) > distance(target, dst))
          h++;

        target = negTeam[i];                              ////////////  Find DOWN-LEFT-path
        for(j = 0; j < mileage; j++)                                //  to a presumed destination named 'target'
          target = dl(target);
        j = 0;                                                      //  Is that path blocked?
        blocked = false;
        next = dl(negTeam[i]);
        while(j < mileage && !blocked && next < _NONE)
          {
            if(opposed(negTeam[i], next, gs))
              blocked = true;
            next = dl(next);
            j++;
          }
                                                                    //  Is the blocked presumed destination
                                                                    //  closer to the true (query) destination?
        if(blocked && distance(negTeam[i], dst) > distance(target, dst))
          h++;

        //////////////////////////////////////////////////////////////
        mileage = backSlashMileage(negTeam[i], gs);    ///////////////  Back-slash mileage
        target = negTeam[i];                           ///////////////  Find UP-LEFT-path
        for(j = 0; j < mileage; j++)                                //  to a presumed destination named 'target'
          target = ul(target);
        j = 0;                                                      //  Is that path blocked?
        blocked = false;
        next = ul(negTeam[i]);
        while(j < mileage && !blocked && next < _NONE)
          {
            if(opposed(negTeam[i], next, gs))
              blocked = true;
            next = ul(next);
            j++;
          }
                                                                    //  Is the blocked presumed destination
                                                                    //  closer to the true (query) destination?
        if(blocked && distance(negTeam[i], dst) > distance(target, dst))
          h++;

        target = negTeam[i];                           ///////////////  Find DOWN-RIGHT-path
        for(j = 0; j < mileage; j++)                                //  to a presumed destination named 'target'
          target = dr(target);
        j = 0;                                                      //  Is that path blocked?
        blocked = false;
        next = dr(negTeam[i]);
        while(j < mileage && !blocked && next < _NONE)
          {
            if(opposed(negTeam[i], next, gs))
              blocked = true;
            next = dr(next);
            j++;
          }
                                                                    //  Is the blocked presumed destination
                                                                    //  closer to the true (query) destination?
        if(blocked && distance(negTeam[i], dst) > distance(target, dst))
          h++;
      }

    return h;
  }

/* Receives a list of potential destinations for negTeam.  */
unsigned char wallsList(unsigned char* negTeam, unsigned char negTeamLen,
                        unsigned char* dstList, unsigned char dstListLen,
                        GameState* gs)
  {
    unsigned char h = 0;
    unsigned char i;
    for(i = 0; i < dstListLen; i++)
      h += walls(negTeam, negTeamLen, dstList[i], gs);
    return h;
  }

/**************************************************************************************************
 Connectedness
 Rewards positions with high connectedness.
 This function looks at the AVERAGE NUMBER OF CONNECTIONS per piece so as to avoid an implicit
 material advantage. */
float connectedness(unsigned char* posTeam, unsigned char posTeamLen, GameState* gs)
  {
    float s = 0.0;
    unsigned char i;
    unsigned char p;

    for(p = 0; p < posTeamLen; p++)
      {
        i = 0;
        if(u(posTeam[p]) < _NONE && sameSide(posTeam[p], u(posTeam[p]), gs))
          i++;
        if(d(posTeam[p]) < _NONE && sameSide(posTeam[p], d(posTeam[p]), gs))
          i++;
        if(l(posTeam[p]) < _NONE && sameSide(posTeam[p], l(posTeam[p]), gs))
          i++;
        if(r(posTeam[p]) < _NONE && sameSide(posTeam[p], r(posTeam[p]), gs))
          i++;
        if(ul(posTeam[p]) < _NONE && sameSide(posTeam[p], ul(posTeam[p]), gs))
          i++;
        if(ur(posTeam[p]) < _NONE && sameSide(posTeam[p], ur(posTeam[p]), gs))
          i++;
        if(dr(posTeam[p]) < _NONE && sameSide(posTeam[p], dr(posTeam[p]), gs))
          i++;
        if(dl(posTeam[p]) < _NONE && sameSide(posTeam[p], dl(posTeam[p]), gs))
          i++;

        s += (float)i;
      }
    return s / (float)posTeamLen;
  }

/**************************************************************************************************
 Uniformity
 Rewards a uniform distribution of pieces.

 The best possible situation: average distance from 1 piece to itself = 0.
 The worst possible situation for 12 pieces on an 8 x 8 board:
 B B . . . . B B  The average position here is (3.5, 3.5).
 B . . . . . . B  The four pieces in the corners are each distant 4.949747468 units.
 . . . . . . . .  The eight shoulder pieces are each distant 4.301162634 units.
 . . . . . . . .  The worst possible average distance is (4 * 4.949747468 + 8 * 4.301162634) / 12 = 4.517357578666666.
 . . . . . . . .  But why don't we just call this 5.0.
 . . . . . . . .  Meaning the best possible value this function can return is 5.0.
 B . . . . . . B  The worst possible value this function can return is a little less than 0.5.
 B B . . . . B B  */
float uniformity(unsigned char* posTeam, unsigned char posTeamLen)
  {
    float h = 0.0;
    float avg_x = 0.0, avg_y = 0.0;
    float x, y;
    unsigned char i;

    for(i = 0; i < posTeamLen; i++)                                 //  Compute "average" position.
      {
        avg_x += (float)col(posTeam[i]);
        avg_y += (float)row(posTeam[i]);
      }
    avg_x /= (float)posTeamLen;
    avg_y /= (float)posTeamLen;

    for(i = 0; i < posTeamLen; i++)                                 //  How far is each piece from this central position?
      {
        x = (float)col(posTeam[i]) - avg_x;
        y = (float)row(posTeam[i]) - avg_y;
        h += sqrt(x * x + y * y);
      }
    h /= (float)posTeamLen;

    return 5.0 - h;
  }

#endif
