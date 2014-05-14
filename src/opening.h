#ifndef CHESS_OPENING_H_INCLUDE
#define CHESS_OPENING_H_INCLUDE
#include "board.h"
#include "moves.h"
#include "pgn.h"
#include "narrator.h"
#define OPENING_TEST_SOURCE "data/test_games.pgn"
#define OPENING_TEST_BINARY "build/test_openings.dat"
#define OPENING_SOURCE "data/master_games.pgn"
#define OPENING_BINARY "build/openings.dat"

typedef struct OpeningBook OpeningBook;

void OpeningBook_generate(char* sourceName, char* outName,
    int maxDepth);
OpeningBook* OpeningBook_load(char* fileName);
void OpeningBook_delete(OpeningBook* self);
int OpeningBook_hasLine(OpeningBook* self, ChessBoard* board,
    int minGames);
move_t OpeningBook_randNextMove(OpeningBook* self, ChessBoard* board, int minGames);
void OpeningBook_print(OpeningBook* self, int minGames);
#endif
