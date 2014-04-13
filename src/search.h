#ifndef CHESS_SEARCH_H_INCLUDE
#define CHESS_SEARCH_H_INCLUDE
#include "board.h"
#define MAX_LINE_LENGTH 64

int getBestLine(ChessBoard* board, int depth, move_t* line, int* lineLength);

#endif
