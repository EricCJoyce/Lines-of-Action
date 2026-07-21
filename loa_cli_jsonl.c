#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>

#include "gamestate.h"
#include "gropius.h"
#include "jsmn.h"

static int tok_eq(const char*, const jsmntok_t*, const char*);
static int tok_copy_string(const char*, const jsmntok_t*, char*, size_t);
static int json_find_top_value(const char*, const jsmntok_t*, int, const char*);
static int hex_nibble(char);
static int hex_decode(const char*, uint8_t[_GAMESTATE_BYTE_SIZE]);
static void hex_encode(const uint8_t*, size_t, char*);
static int move_hex_decode_2(const char*, Move*);
static void move_hex_encode_2(Move*, char[_MOVE_BYTE_SIZE*2+1]);
void serialize(GameState*, unsigned char*);
void deserialize(const uint8_t*, GameState*);

//  echo '{"cmd":"startpos"}' | ./loa_cli
//  echo '{"cmd":"draw","state_hex":"7e0000000000007e008181818181810080"}' | ./loa_cli
//  echo '{"cmd":"features","state_hex":"7e0000000000007e008181818181810080"}' | ./loa_cli

//  echo '{"cmd":"legal_moves","state_hex":"7e0000000000007e008181818181810080"}' | ./loa_cli
//  echo '{"cmd":"apply_move","state_hex":"7e0000000000007e008181818181810080","move_hex":"0111"}' | ./loa_cli

//  echo '{"cmd":"draw","state_hex":"3e0040000000007e008181818181810000"}' | ./loa_cli
//  echo '{"cmd":"features","state_hex":"3e0040000000007e008181818181810000"}' | ./loa_cli

static int handle_request(const char* line);

int main(void)
  {
    char line[4096];

    while(fgets(line, sizeof(line), stdin))
      {
        int rc = handle_request(line);
        fflush(stdout);
        fflush(stderr);
        if(rc != 0)
          return rc;
      }

    return 0;
  }

static int handle_request(const char* line)
  {
    unsigned int i;
    int ntok;
    jsmn_parser p;                                                  //  Parse JSON.
    jsmntok_t toks[256];
    int cmd_i;
    char cmd[64];
    uint8_t state[_GAMESTATE_BYTE_SIZE];
    uint8_t next_state[_GAMESTATE_BYTE_SIZE];
    char hex[2 * _GAMESTATE_BYTE_SIZE + 1];
    char state_hex[2 * _GAMESTATE_BYTE_SIZE + 1];
    int st_i;
    GameState gs;

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
    float f[10];

    int is_term;
    int res;

    Move moves[_MAX_MOVES];
    unsigned int movesLen = 0;
    char mh[_MOVE_BYTE_SIZE*2+1];
    int mv_i;

    char move_hex[_MOVE_BYTE_SIZE*2+1];
    Move mv;
    float piece_total;
    float total;

    const uint8_t startpos_loa[] = {126,0,0,0,0,0,0,126,0,129,129,129,129,129,129,0,128};
    const uint8_t startpos_scrambledeggs[] = {84,1,128,1,128,1,128,42,42,128,1,128,1,128,1,84,128};

    //////////////////////////////////////////////////////////////////

    srand(time(NULL));
    jsmn_init(&p);
    ntok = jsmn_parse(&p, line, (int)strlen(line), toks, (int)(sizeof(toks)/sizeof(toks[0])));
    if(ntok < 0)
      {
        fprintf(stderr, "JSON parse error: %d\n", ntok);
        return 3;
      }

    cmd_i = json_find_top_value(line, toks, ntok, "cmd");           //  cmd
    if(cmd_i < 0 || toks[cmd_i].type != JSMN_STRING)
      {
        fprintf(stderr, "Missing/invalid 'cmd'.\n");
        return 4;
      }

    if(!tok_copy_string(line, &toks[cmd_i], cmd, sizeof(cmd)))
      {
        fprintf(stderr, "'cmd' too long.\n");
        return 4;
      }

    if(strcmp(cmd, "startpos") == 0)
      {
        switch( rand() % 2 )
          {
            case 1:
              for(i = 0; i < _GAMESTATE_BYTE_SIZE; i++)
                state[i] = startpos_scrambledeggs[i];
              break;

            default:
              for(i = 0; i < _GAMESTATE_BYTE_SIZE; i++)
                state[i] = startpos_loa[i];
          }
        hex_encode(state, _GAMESTATE_BYTE_SIZE, hex);               //  Write the start set.
        printf("{\"state_hex\":\"%s\"}\n", hex);
        return 0;
      }

    if(strcmp(cmd, "print_move") == 0)
      {
        mv_i = json_find_top_value(line, toks, ntok, "move_hex");
        if(mv_i < 0 || toks[mv_i].type != JSMN_STRING)
          {
            fprintf(stderr, "Missing/invalid 'move_hex'.\n");
            return 6;
          }

        if(!tok_copy_string(line, &toks[mv_i], move_hex, sizeof(move_hex)))
          {
            fprintf(stderr, "'move_hex' wrong length.\n");
            return 6;
          }

        if(!move_hex_decode_2(move_hex, &mv))
          {
            fprintf(stderr, "Bad hex in 'move_hex'.\n");
            return 6;
          }

        printf("{\"move_from\":%d, \"move_to\":%d}\n", mv.from, mv.to);

        return 0;
      }

    st_i = json_find_top_value(line, toks, ntok, "state_hex");      //  All other commands require state_hex.
    if(st_i < 0 || toks[st_i].type != JSMN_STRING)
      {
        fprintf(stderr, "Missing/invalid 'state_hex'.\n");
        return 5;
      }

    if(!tok_copy_string(line, &toks[st_i], state_hex, sizeof(state_hex)))
      {
        fprintf(stderr, "'state_hex' wrong length.\n");
        return 5;
      }

    if(!hex_decode(state_hex, state))
      {
        fprintf(stderr, "Bad hex in 'state_hex'.\n");
        return 5;
      }

    memset(&gs, 0, sizeof(gs));
    deserialize(state, &gs);

    if(strcmp(cmd, "draw") == 0)
      {
        printf("{");
        printf("\"row_8\":\"");
        for(i = 56; i < _NONE; i++)
          {
            switch(gs.board[i])
              {
                case _EMPTY:         printf(".");  break;
                case _BLACK_PAWN:    printf("B");  break;
                case _WHITE_PAWN:    printf("W");  break;
              }
          }
        printf("\",");

        printf("\"row_7\":\"");
        for(i = 48; i < 56; i++)
          {
            switch(gs.board[i])
              {
                case _EMPTY:         printf(".");  break;
                case _BLACK_PAWN:    printf("B");  break;
                case _WHITE_PAWN:    printf("W");  break;
              }
          }
        printf("\",");

        printf("\"row_6\":\"");
        for(i = 40; i < 48; i++)
          {
            switch(gs.board[i])
              {
                case _EMPTY:         printf(".");  break;
                case _BLACK_PAWN:    printf("B");  break;
                case _WHITE_PAWN:    printf("W");  break;
              }
          }
        printf("\",");

        printf("\"row_5\":\"");
        for(i = 32; i < 40; i++)
          {
            switch(gs.board[i])
              {
                case _EMPTY:         printf(".");  break;
                case _BLACK_PAWN:    printf("B");  break;
                case _WHITE_PAWN:    printf("W");  break;
              }
          }
        printf("\",");

        printf("\"row_4\":\"");
        for(i = 24; i < 32; i++)
          {
            switch(gs.board[i])
              {
                case _EMPTY:         printf(".");  break;
                case _BLACK_PAWN:    printf("B");  break;
                case _WHITE_PAWN:    printf("W");  break;
              }
          }
        printf("\",");

        printf("\"row_3\":\"");
        for(i = 16; i < 24; i++)
          {
            switch(gs.board[i])
              {
                case _EMPTY:         printf(".");  break;
                case _BLACK_PAWN:    printf("B");  break;
                case _WHITE_PAWN:    printf("W");  break;
              }
          }
        printf("\",");

        printf("\"row_2\":\"");
        for(i = 8; i < 16; i++)
          {
            switch(gs.board[i])
              {
                case _EMPTY:         printf(".");  break;
                case _BLACK_PAWN:    printf("B");  break;
                case _WHITE_PAWN:    printf("W");  break;
              }
          }
        printf("\",");

        printf("\"row_1\":\"");
        for(i = 0; i < 8; i++)
          {
            switch(gs.board[i])
              {
                case _EMPTY:         printf(".");  break;
                case _BLACK_PAWN:    printf("B");  break;
                case _WHITE_PAWN:    printf("W");  break;
              }
          }
        printf("\",");

        if(gs.blackToMove)
          printf("\"black_to_move\":true}\n");
        else
          printf("\"black_to_move\":false}\n");

        return 0;
      }

    if(strcmp(cmd, "features") == 0)
      {
        //////////////////////////////////////////////////////////////  Compute the following only ONCE.
        blackMaterialLength = getBlack(&gs, blackMaterial);         //  unsigned chars
        whiteMaterialLength = getWhite(&gs, whiteMaterial);         //  unsigned chars

        blackMovesLength = getMovesForTeam(true, &gs, blackMoves);  //  Moves
        whiteMovesLength = getMovesForTeam(false, &gs, whiteMoves); //  Moves

        blackCoM = centerOfMass(blackMaterial, blackMaterialLength);//  Index
        whiteCoM = centerOfMass(whiteMaterial, whiteMaterialLength);//  Index

        if(gs.blackToMove)
          {
            f[0] = concentration(blackMaterial, blackMaterialLength) - concentration(whiteMaterial, whiteMaterialLength);
            f[1] = centralization(blackMaterial, blackMaterialLength) - centralization(whiteMaterial, whiteMaterialLength);
            f[2] = evaluateCenterOfMass(blackCoM) - evaluateCenterOfMass(whiteCoM);
            f[3] = quads(blackMaterial, blackMaterialLength, blackCoM, &gs) - quads(whiteMaterial, whiteMaterialLength, whiteCoM, &gs);
            f[4] = mobility(blackMoves, blackMovesLength, blackMaterialLength, &gs) - mobility(whiteMoves, whiteMovesLength, whiteMaterialLength, &gs);
            f[5] = wallsCOM(whiteMaterial, whiteMaterialLength, whiteCoM, &gs) - wallsCOM(blackMaterial, blackMaterialLength, blackCoM, &gs);
            f[6] = wallsCenter4(whiteMaterial, whiteMaterialLength, &gs) - wallsCenter4(blackMaterial, blackMaterialLength, &gs);
            f[7] = wallsCenter12(whiteMaterial, whiteMaterialLength, &gs) - wallsCenter12(blackMaterial, blackMaterialLength, &gs);
            f[8] = connectedness(blackMaterial, blackMaterialLength, &gs) - connectedness(whiteMaterial, whiteMaterialLength, &gs);
            f[9] = uniformity(blackMaterial, blackMaterialLength) - uniformity(whiteMaterial, whiteMaterialLength);
          }
        else
          {
            f[0] = concentration(whiteMaterial, whiteMaterialLength) - concentration(blackMaterial, blackMaterialLength);
            f[1] = centralization(whiteMaterial, whiteMaterialLength) - centralization(blackMaterial, blackMaterialLength);
            f[2] = evaluateCenterOfMass(whiteCoM) - evaluateCenterOfMass(blackCoM);
            f[3] = quads(whiteMaterial, whiteMaterialLength, whiteCoM, &gs) - quads(blackMaterial, blackMaterialLength, blackCoM, &gs);
            f[4] = mobility(whiteMoves, whiteMovesLength, whiteMaterialLength, &gs) - mobility(blackMoves, blackMovesLength, blackMaterialLength, &gs);
            f[5] = wallsCOM(blackMaterial, blackMaterialLength, blackCoM, &gs) - wallsCOM(whiteMaterial, whiteMaterialLength, whiteCoM, &gs);
            f[6] = wallsCenter4(blackMaterial, blackMaterialLength, &gs) - wallsCenter4(whiteMaterial, whiteMaterialLength, &gs);
            f[7] = wallsCenter12(blackMaterial, blackMaterialLength, &gs) - wallsCenter12(whiteMaterial, whiteMaterialLength, &gs);
            f[8] = connectedness(whiteMaterial, whiteMaterialLength, &gs) - connectedness(blackMaterial, blackMaterialLength, &gs);
            f[9] = uniformity(whiteMaterial, whiteMaterialLength) - uniformity(blackMaterial, blackMaterialLength);
          }

        printf("{\"features\":[%.9g,%.9g,%.9g,%.9g,%.9g,%.9g,%.9g,%.9g,%.9g,%.9g]}\n",
               f[0],f[1],f[2],f[3],f[4],f[5],f[6],f[7],f[8],f[9]);
        return 0;
      }

    if(strcmp(cmd, "terminal") == 0)
      {
        is_term = terminal(&gs) ? 1 : 0;

        if(!is_term)
          {
            printf("{\"terminal\":false}\n");
          }
        else
          {
            res = isWin(&gs);
            if((res == GAME_OVER_BLACK_WINS && !gs.blackToMove) || (res == GAME_OVER_WHITE_WINS && gs.blackToMove))
              printf("{\"terminal\":true,\"result\":\"loss\"}\n");
            else if(res == GAME_OVER_DRAW)
              printf("{\"terminal\":true,\"result\":\"draw\"}\n");
            else
              printf("{\"terminal\":true,\"result\":\"win\"}\n");
          }
        return 0;
      }

    if(strcmp(cmd, "legal_moves") == 0)
      {
        movesLen = getMoves(&gs, moves);

        printf("{\"moves_hex\":[");                                 //  Emit JSON.
        for(i = 0; i < movesLen; i++)
          {
            move_hex_encode_2(moves + i, mh);
            printf("\"%s\"%s", mh, (i + 1 < movesLen) ? "," : "");
          }
        printf("]}\n");
        return 0;
      }

    if(strcmp(cmd, "apply_move") == 0)
      {
        mv_i = json_find_top_value(line, toks, ntok, "move_hex");
        if(mv_i < 0 || toks[mv_i].type != JSMN_STRING)
          {
            fprintf(stderr, "Missing/invalid 'move_hex'.\n");
            return 6;
          }

        if(!tok_copy_string(line, &toks[mv_i], move_hex, sizeof(move_hex)))
          {
            fprintf(stderr, "'move_hex' wrong length.\n");
            return 6;
          }

        if(!move_hex_decode_2(move_hex, &mv))
          {
            fprintf(stderr, "Bad hex in 'move_hex'.\n");
            return 6;
          }

        makeMove(&mv, &gs);
        serialize(&gs, next_state);

        hex_encode(next_state, _GAMESTATE_BYTE_SIZE, hex);
        printf("{\"state_hex\":\"%s\"}\n", hex);
        return 0;
      }

    fprintf(stderr, "Unknown cmd: %s\n", cmd);
    return 7;
  }

//  Compare a token to a literal string (token is not null-terminated).
static int tok_eq(const char* json, const jsmntok_t* tok, const char* s)
  {
    int len = (int)strlen(s);
    int tlen = tok->end - tok->start;
    return (tok->type == JSMN_STRING && tlen == len && strncmp(json + tok->start, s, (size_t)len) == 0);
  }

//  Copy token string into out (null-terminated). Returns 1 on success.
static int tok_copy_string(const char* json, const jsmntok_t* tok, char* out, size_t out_cap)
  {
    int tlen = tok->end - tok->start;
    if((size_t)tlen + 1 > out_cap)
      return 0;
    memcpy(out, json + tok->start, (size_t)tlen);
    out[tlen] = '\0';
    return 1;
  }

//  Find the token index of value for a given key in the top-level object.
//  Returns value token index, or -1 if not found.
static int json_find_top_value(const char* json, const jsmntok_t* toks, int ntok, const char* key)
  {
    if(ntok < 1 || toks[0].type != JSMN_OBJECT)
      return -1;
                                                                    //  jsmn stores object as: { key, value, key, value, ... } in tokens after toks[0]
    int i = 1;
    int pairs = toks[0].size;
    for(int p = 0; p < pairs; p++)
      {
        const jsmntok_t *k = &toks[i];
        const jsmntok_t *v = &toks[i + 1];
        if(tok_eq(json, k, key))
          {
            return i + 1;
          }
                                                                    //  Advance i to next key. BUT: value can be an object/array with nested tokens.
                                                                    //  We need to skip over the entire value subtree.
        i += 2;
                                                                    //  If v is a primitive/string, skipping is already done.
                                                                    //  If v is object/array, skip its nested tokens:
        if(v->type == JSMN_OBJECT || v->type == JSMN_ARRAY)
          {
                                                                    //  Skip over all descendant tokens (simple walker).
            int to_skip = 1;
            while(to_skip > 0 && i < ntok)
              {
                if(toks[i].type == JSMN_OBJECT || toks[i].type == JSMN_ARRAY)
                  {
                    to_skip += toks[i].size;
                  }
                to_skip--;
                i++;
              }
          }
      }
    return -1;
  }

static int hex_nibble(char c)
  {
    if('0' <= c && c <= '9')
      return c - '0';
    c = (char)tolower((unsigned char)c);
    if('a' <= c && c <= 'f')
      return 10 + (c - 'a');
    return -1;
  }

static int hex_decode(const char* hex, uint8_t out[_GAMESTATE_BYTE_SIZE])
  {
    if(!hex)
      return 0;
    if(strlen(hex) != (_GAMESTATE_BYTE_SIZE * 2))
      return 0;
    for(int i = 0; i < _GAMESTATE_BYTE_SIZE; i++)
      {
        int hi = hex_nibble(hex[2 * i]);
        int lo = hex_nibble(hex[2 * i + 1]);
        if(hi < 0 || lo < 0)
          return 0;
        out[i] = (uint8_t)((hi << 4) | lo);
      }
    return 1;
  }

static void hex_encode(const uint8_t* in, size_t n, char* out_hex)
  {
    static const char *digits = "0123456789abcdef";
    for(size_t i = 0; i < n; i++)
      {
        out_hex[2 * i]     = digits[(in[i] >> 4) & 0xF];
        out_hex[2 * i + 1] = digits[in[i] & 0xF];
      }
    out_hex[2 * n] = '\0';
  }

static int move_hex_decode_2(const char *hex, Move* mv)
  {
    uint8_t out_move[2];
    if(!hex)
      return 0;
    if(strlen(hex) != 4)
      return 0;
    for(int i = 0; i < 2; i++)
      {
        int hi = hex_nibble(hex[2*i]);
        int lo = hex_nibble(hex[2*i + 1]);
        if (hi < 0 || lo < 0) return 0;
        out_move[i] = (uint8_t)((hi << 4) | lo);
      }
    mv->from = out_move[0];
    mv->to = out_move[1];
    return 1;
  }

static void move_hex_encode_2(Move* mv, char out_hex[_MOVE_BYTE_SIZE*2+1])
  {
    static const char *d = "0123456789abcdef";

    out_hex[0] = d[(mv->from >> 4) & 0xF];
    out_hex[1] = d[mv->from & 0xF];

    out_hex[2] = d[(mv->to >> 4) & 0xF];
    out_hex[3] = d[mv->to & 0xF];

    out_hex[4] = '\0';
  }

/* Pack a GameState into the unsigned-char buffer. */
void serialize(GameState* gs, unsigned char* buffer)
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

/* Recover a GameState from the unsigned-char buffer". */
void deserialize(const uint8_t* buffer, GameState* gs)
  {
    unsigned char x, y;
    unsigned char i = 0;
    unsigned char ch, mask;

    for(y = 0; y < 8; y++)                                          //  (8 bytes) Decode black.
      {
        ch = buffer[i++];
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
        ch = buffer[i++];
        mask = 128;
        for(x = 0; x < 8; x++)
          {
            if((ch & mask) == mask)
              gs->board[y * 8 + x] = _WHITE_PAWN;
            mask >>= 1;
          }
      }

    gs->blackToMove = ((buffer[i] & 128) == 128);                   //  (1 byte) Decode side to move.

    return;                                                         //  TOTAL: 17 bytes.
  }
