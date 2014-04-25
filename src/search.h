#ifndef CHESS_SEARCH_H_INCLUDE
#define CHESS_SEARCH_H_INCLUDE
#include "board.h"
#include "threads.h"
#include <time.h>
#define MAX_LINE_LENGTH 64

typedef enum {OPENING=0, MIDGAME=1, ENDGAME=2, PUZZLE=3} searchType_e;

typedef struct SearchThread SearchThread;

SearchThread* SearchThread_new(ChessBoard* board);
void SearchThread_delete(SearchThread* self);
void SearchThread_setSearchType(SearchThread* self, searchType_e type);
searchType_e SearchThread_getSearchType(SearchThread* self);
void SearchThread_setTimeout(SearchThread* self, time_t max_seconds);
long SearchThread_getTimeout(SearchThread* self);
int SearchThread_getBestLine(SearchThread* self, move_t* lineOut, int* lineLength);
void SearchThread_start(SearchThread* self);

#endif
