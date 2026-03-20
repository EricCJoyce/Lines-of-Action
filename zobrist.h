#ifndef __ZOBRIST_H
#define __ZOBRIST_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define ZHASH_TABLE_SIZE    129

#define _BLACK_PAWN        0x01
#define _WHITE_PAWN        0x02

#define B_A1                  0
#define B_B1                  1
#define B_C1                  2
#define B_D1                  3
#define B_E1                  4
#define B_F1                  5
#define B_G1                  6
#define B_H1                  7
#define B_A2                  8
#define B_B2                  9
#define B_C2                 10
#define B_D2                 11
#define B_E2                 12
#define B_F2                 13
#define B_G2                 14
#define B_H2                 15
#define B_A3                 16
#define B_B3                 17
#define B_C3                 18
#define B_D3                 19
#define B_E3                 20
#define B_F3                 21
#define B_G3                 22
#define B_H3                 23
#define B_A4                 24
#define B_B4                 25
#define B_C4                 26
#define B_D4                 27
#define B_E4                 28
#define B_F4                 29
#define B_G4                 30
#define B_H4                 31
#define B_A5                 32
#define B_B5                 33
#define B_C5                 34
#define B_D5                 35
#define B_E5                 36
#define B_F5                 37
#define B_G5                 38
#define B_H5                 39
#define B_A6                 40
#define B_B6                 41
#define B_C6                 42
#define B_D6                 43
#define B_E6                 44
#define B_F6                 45
#define B_G6                 46
#define B_H6                 47
#define B_A7                 48
#define B_B7                 49
#define B_C7                 50
#define B_D7                 51
#define B_E7                 52
#define B_F7                 53
#define B_G7                 54
#define B_H7                 55
#define B_A8                 56
#define B_B8                 57
#define B_C8                 58
#define B_D8                 59
#define B_E8                 60
#define B_F8                 61
#define B_G8                 62
#define B_H8                 63
#define W_A1                 64
#define W_B1                 65
#define W_C1                 66
#define W_D1                 67
#define W_E1                 68
#define W_F1                 69
#define W_G1                 70
#define W_H1                 71
#define W_A2                 72
#define W_B2                 73
#define W_C2                 74
#define W_D2                 75
#define W_E2                 76
#define W_F2                 77
#define W_G2                 78
#define W_H2                 79
#define W_A3                 80
#define W_B3                 81
#define W_C3                 82
#define W_D3                 83
#define W_E3                 84
#define W_F3                 85
#define W_G3                 86
#define W_H3                 87
#define W_A4                 88
#define W_B4                 89
#define W_C4                 90
#define W_D4                 91
#define W_E4                 92
#define W_F4                 93
#define W_G4                 94
#define W_H4                 95
#define W_A5                 96
#define W_B5                 97
#define W_C5                 98
#define W_D5                 99
#define W_E5                100
#define W_F5                101
#define W_G5                102
#define W_H5                103
#define W_A6                104
#define W_B6                105
#define W_C6                106
#define W_D6                107
#define W_E6                108
#define W_F6                109
#define W_G6                110
#define W_H6                111
#define W_A7                112
#define W_B7                113
#define W_C7                114
#define W_D7                115
#define W_E7                116
#define W_F7                117
#define W_G7                118
#define W_H7                119
#define W_A8                120
#define W_B8                121
#define W_C8                122
#define W_D8                123
#define W_E8                124
#define W_F8                125
#define W_G8                126
#define W_H8                127

#define B_TO_MOVE           128

/**************************************************************************************************
 Typedefs  */


/**************************************************************************************************
 Prototypes  */


/**************************************************************************************************
 Globals  */
                                                                    //  1,032 bytes.
unsigned char zobristHashBuffer[ZHASH_TABLE_SIZE * 8];              //  Global array containing the serialized Zobrist-hasher values (unsigned long longs).
                                                                    //  "Keys" are simply unisnged int values #defined above.

/* Because it indexes into "zobristHashBuffer", the hash function is defined in negamax.cpp. */

#endif