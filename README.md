# [Lines-of-Action](https://www.ericjoycefilm.com/wastesoftime/boardgames/linesofaction/index.php?lang=en)
Notes on the creation of Lines of Action

## Docker container to compile C to WebAssembly
Create the container.
```
sudo docker build -t emscripten-c .
```

Confirm its existence.
```
sudo docker images
```

Kill the container.
```
sudo docker image rm emscripten-c
```

## Zobrist hash generator
This executable (not a WebAssembly module) lives on the server back-end. Compile using GCC. Call it when the page loads to generate a random Zobrist hash for every game.
```
gcc -Wall zgenerate.c -lm -o zgenerate
```

## Opening-book Zobrist hasher
Unlike the in-game hasher, this one is *not* randomly generated for each session.

This executable (not a WebAssembly module) lives on the server back-end. Compile using GCC. Call it from the PHP lookup script.
```
g++ -Wall hash.cpp -lm -o hash
```

For example:
```
./hash 126 0 0 0 0 0 0 126 0 129 129 129 129 129 129 0 128
```

should produce
```
5317689825764910829
```

To look this position up in the opening book, call:
```
./lookup 5317689825764910829 126 0 0 0 0 0 0 126 0 129 129 129 129 129 129 0 128
```

which produces, for example,
```
SUCCESS,3,17
```

which is the one of four moves on file for this state.

## Constants for the chess engine

| Name  | Bytes  | Description |
| :---:	| :----: | :---------: |
| _GAMESTATE_BYTE_SIZE | 17 | Number of bytes needed to encode a game state |
| _MOVE_BYTE_SIZE | 2 | Number of bytes needed to describe a move in Chess |
| _MAX_NUM_TARGETS | 32 | A (generous) upper bound on how many distinct destinations (not distinct moves) may be available to a player from a single index |
| _MAX_MOVES | 128 | A (generous) upper bound on how many moves may be made by a team in a single turn |
| _PARAMETER_ARRAY_SIZE | 16 | Encodes values that are written to and read from the the search process |
| _KILLER_MOVE_PER_PLY | 2 | Chess engines typically store 2 killer moves per ply |
| _KILLER_MOVE_MAX_DEPTH | 64 | Not to say that we actually search to depth 64! This is just comfortably large. |
| _TRANSPO_RECORD_BYTE_SIZE | 17 | Number of bytes needed to store a TranspoRecord object |
| _TRANSPO_TABLE_SIZE | 524288 | Number of TranspoRecords, each 18 bytes |
| _TREE_SEARCH_ARRAY_SIZE | 65536 | Number of (game-state bytes, move-bytes) |
| _NEGAMAX_NODE_BYTE_SIZE | 69 | Number of bytes needed to encode a negamax node |
| _NEGAMAX_MOVE_BYTE_SIZE | 3 | Number of bytes needed to encode a negamax move (in their separate, global array) |
| ZHASH_TABLE_SIZE | 129 | Number of Zobrist keys |
| _BLACK_TO_MOVE | 0 | Indication that black is to move in the current game state |
| _WHITE_TO_MOVE | 1 | Indication that white is to move in the current game state |

## Client-facing game logic module

### Game-Logic Module

![Game Logic Schema](Game_Logic_Schema.png)

The **game-logic module** has *two* outward-facing buffers:
- `currentState` is `_GAMESTATE_BYTE_SIZE` bytes long. It encodes the current state of the game.
- `movesBuffer` is `_MAX_NUM_TARGETS` bytes long.

Compile the front-end, client-facing game-logic module. This WebAssembly module answers queries from the client-side like getting data about which pieces can move where.
```
sudo docker run --rm -v $(pwd):/src -u $(id -u):$(id -g) --mount type=bind,source=$(pwd),target=/home/src emscripten-c emcc -Os -s STANDALONE_WASM -s EXPORTED_FUNCTIONS="['_getCurrentState','_getMovesBuffer','_sideToMove_client','_isBlack_client','_isWhite_client','_isEmpty_client','_getMovesIndex_client','_makeMove_client','_isTerminal_client','_isWin_client','_draw']" -Wl,--no-entry "gamelogic.c" -o "gamelogic.wasm"
```

This produces a `.wasm` with callable functions.

The first two simply fetch the memory addresses of this module's buffers:
- `gameEngine.instance.exports.getCurrentState();` returns the address of the game-logic module's current-gamestate buffer.
- `gameEngine.instance.exports.getMovesBuffer();` returns the address of the game-logic module's move-targets buffer.

The other module functions are as follows:
- `gameEngine.instance.exports.sideToMove_client();` returns `_BLACK_TO_MOVE` or `_WHITE_TO_MOVE`.
- `gameEngine.instance.exports.isBlack_client(unsigned char);` returns a Boolean value indicating whether the piece at the given index belongs to the black team.
- `gameEngine.instance.exports.isWhite_client(unsigned char);` returns a Boolean value indicating whether the piece at the given index belongs to the white team.
- `gameEngine.instance.exports.isEmpty_client(unsigned char);` returns a Boolean value indicating whether the given index is empty.
- `gameEngine.instance.exports.getMovesIndex_client(unsigned char);` returns the number of legal targets `n` as an unsigned int and writes a run of `n` indices to the move-targets buffer.
- `gameEngine.instance.exports.makeMove_client(unsigned char, unsigned char, unsigned char);` applies the given move to the current game state and overwrites its encoding in the `currentState` buffer.
- `gameEngine.instance.exports.isTerminal_client();` returns a Boolean value indicating whether the current game state is terminal.
- `gameEngine.instance.exports.isWin_client();` returns an unsigned char indicating whether white has won, black has won, the game has reached stalemate, or the game is ongoing.
- `gameEngine.instance.exports.draw();` prints the board to the browser console.

## Citation
If this code was helpful to you, please cite this repository.

```
@misc{linesofaction,
  title={Lines of Action in C},
  author={Eric C. Joyce},
  year={2025},
  publisher={Github},
  journal={GitHub repository},
  howpublished={\url{https://github.com/EricCJoyce/Lines-of-Action}}
}
```
