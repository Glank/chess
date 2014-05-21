#ifndef CHESS_MIND_H_INCLUDE
#define CHESS_MIND_H_INCLUDE
#include "board.h"
#include "threads.h"
#include "heuristics.h"
#include "moves.h"
#include "opening.h"
#include "search.h"
typedef struct ChessMind ChessMind;

ChessMind* ChessMind_new(ChessBoard* board);
void ChessMind_delete(ChessMind* self);
void ChessMind_setTimeout(ChessMind* self, int timeout);
//search without ever timing out, print each search thread
void ChessMind_puzzleSearch(ChessMind* self);
//search until timeout or use opening book
void ChessMind_makeMove(ChessMind* self);
void ChessMind_startPondering(ChessMind* self);
void ChessMind_stopPondering(ChessMind* self);
#endif
