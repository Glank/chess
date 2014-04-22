#ifndef CHESS_SEARCH_H_INCLUDE
#define CHESS_SEARCH_H_INCLUDE
#include "board.h"
#include "threads.h"
#define MAX_LINE_LENGTH 64

typedef enum {OPENING, MIDGAME, ENDGAME, PUZZLE} searchType_e;

typedef struct SearchThread SearchThread;

SearchThread* SearchThread_new(ChessBoard* board, searchType_e type);
void SearchThread_delete(SearchThread* self);
void SearchThread_setTimeout(SearchThread* self, long max_milliseconds);
long SearchThread_getTimeout(SearchThread* self);
void SearchThread_start(SearchThread* self);
int SearchThread_getBestLine(SearchThread* self, move_t* lineOut, int* lineLength);

#endif
