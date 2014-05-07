#ifndef CHESS_SEARCH_H_INCLUDE
#define CHESS_SEARCH_H_INCLUDE
#include "board.h"
#include "threads.h"
#include "heuristics.h"
#include "moves.h"
#include <time.h>
#define MAX_LINE_LENGTH 64
#define TTABLE_SIZE 2048

typedef enum {OPENING=0, MIDGAME=1, ENDGAME=2, PUZZLE=3} searchType_e;

typedef struct SearchThread SearchThread;
typedef struct TTable TTable;
typedef struct TNode TNode;

struct TNode{
    zob_hash_t hash;
    int evaluation;
    int depth;
    int isCut;
    int halfMoveNumber;
};
struct TTable{
    TNode nodes[TTABLE_SIZE];
    TNode* sortingRoom[MOVE_GEN_MAX_ALLOCATED];
    ChessHNode* hMergeRoom[MOVE_GEN_MAX_ALLOCATED];
    TNode* tMergeRoom[MOVE_GEN_MAX_ALLOCATED];
    int minHalfMoveNumber;
};
TTable* TTable_new();
void TTable_delete(TTable* self);

struct SearchThread{
    ChessThread* thread;
    ChessMutex* bestLineMutex;
    int runFlag; //1 = continue, 0 = stop ASAP
    time_t max_seconds;
    time_t start_time;

    ChessHEngine* engine;
    TTable* table;
    ChessBoard* board;
    searchType_e searchType;

    move_t bestLine[MAX_LINE_LENGTH];
    int bestLineLength;
    int bestLineValue;
    int printEachNewLine;
};
SearchThread* SearchThread_new(ChessBoard* board, TTable* table);
void SearchThread_delete(SearchThread* self);
void SearchThread_setSearchType(SearchThread* self, searchType_e type);
searchType_e SearchThread_getSearchType(SearchThread* self);
void SearchThread_setTimeout(SearchThread* self, time_t max_seconds);
long SearchThread_getTimeout(SearchThread* self);
int SearchThread_getBestLine(SearchThread* self, move_t* lineOut, int* lineLength);
void SearchThread_setPrintEachNewLine(SearchThread* self, int b);
int SearchThread_getPrintEachNewLine(SearchThread* self);
void SearchThread_printBestLine(SearchThread* self);
void SearchThread_start(SearchThread* self);
void SearchThread_stop(SearchThread* self);
void SearchThread_join(SearchThread* self);
int SearchThread_isRunning(SearchThread* self);
move_t SearchThread_getBestMove(SearchThread* self);

#endif
