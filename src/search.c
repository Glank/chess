#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <limits.h>
#include "search.h"
#include "heuristics.h"
#define MAX_DEPTH 8

const int DEPTH_ORDERS[][] = {
    {1,2,3,4,5,6,7,8,9}, //opening depths
    {1,2,2,3,3,3,4,4,4}, //midgame depths
    {1,2,2,3,3,4,4,5,5}, //endgame depths
    {1,2,2,3,3,3,4,4,4}  //puzzle  depths
};
const int QUIECENSE_ORDERS[][] = {
    {0,0,1,1,2,2,3,3,4}, //opening depths
    {0,0,1,1,2,2,3,3,4}, //midgame depths
    {0,0,1,1,2,2,3,3,4}, //endgame depths
    {0,0,1,1,2,2,3,3,4}  //puzzle  depths
};
const int DEEP_QUIECENSE_ORDERS[][] = {
    {0,0,1,1,2,2,3,3,4}, //opening depths
    {0,0,1,1,2,2,3,3,4}, //midgame depths
    {0,0,1,1,2,2,3,3,4}, //endgame depths
    {0,0,1,1,2,2,3,3,4}  //puzzle  depths
};

void* searchMain(void* args);
int alphabeta(
    SearchThread* self, ChessHNode* node, 
    int depth, int quiecense, int deepQuiecense,
    int alpha, int beta, int* runFlag,
    move_t* lineout, int* lineoutLength);
int isDying(SearchThread* self);

struct SearchThread{
    ChessThread* thread;
    ChessMutex* bestLineMutex;
    int runFlag; //1 = continue, 0 = stop ASAP
    time_t max_seconds;
    time_t start_time;

    ChessMoveGenerator* gen;
    ChessBoard* board;
    searchType_e searchType;

    move_t* bestLine;
    int bestLineLength;
    int bestLineValue;
};
SearchThread* SearchThread_new(ChessBoard* board){
    SearchThread* self = (SearchThread*)malloc(sizeof(SearchThread));
    self->thread = ChessThread_new(&searchMain);
    self->bestLineMutex = ChessMutex_new();
    self->max_seconds = 60L;

    self->gen = ChessMoveGenerator_new(board);
    self->board = board;
    self->searchType = PUZZLE;

    self->bestLine = (move_t*)malloc(sizeof(move_t)*MAX_LINE_LENGTH);
    self->bestLineLength = 0;
    self->bestLineValue = 0;
}
void SearchThread_delete(SearchThread* self){
    ChessMoveGenerator_delete(self->gen);
    ChessThread_delete(self->thread);
    ChessMutex_delete(self->bestLineMutex);
}
void SearchThread_setSearchType(SearchThread* self, searchType_e type){
    self->searchType = type;
}
searchType_e SearchThread_getSearchType(SearchThread* self){
    return self->searchType;
}
void SearchThread_setTimeout(SearchThread* self, time_t max_milliseconds){
    self->max_milliseconds = max_milliseconds;
}
long SearchThread_getTimeout(SearchThread* self){
    return self->max_milliseconds;
}
int SearchThread_getBestLine(SearchThread* self, move_t* lineOut, int* lineLength){
    int ret;
    ChessMutex_lock(self->bestLineMutex);
        int i;
        for(i=0; i<self->bestLineValue; i++)
            lineOut[i] = self->bestLine[i];
        (*lineLength) = self->bestLineLength;
        ret = self->bestLineValue;
    ChessMutex_unlock(self->bestLineMutex);
    return ret;
}

int isDying(SearchThread* self){
    return ((!self->runFlag)||(self->max_time < time(NULL)-self->start_time));
}

int alphabeta(
    SearchThread* self, ChessHNode* node, 
    int depth, int quiecense, int deepQuiecense,
    int alpha, int beta, int* runFlag,
    move_t* lineout, int* lineoutLength){
    if(isDying(self)){
        (*lineoutLength) = 0;
        return 0;
    }
    //quiecense extencions
    if(depth==0){
        int delta = (node->evaluation)-(node->parent->evaluation);
        delta = delta<0?-delta:delta;
        if(node->inCheck)
            delta = INT_MAX;
        else if(node->parent != NULL && node->parent->inCheck)
            delta = INT_MAX;
        if(delta>=100 && quiecense){
            depth++; //not quiet
            quiecense--;
        }
        else if(delta>=200 && deepQuiecense){
            depth++; //not even kindof quiet
            deepQuiecense--;
        }
        else{
            *lineoutLength = 0;
            return node->evaluation;
        }
    }
    //expand
    ChessHNode_expandBranches(node, gen);
    //if terminal
    if(node->childrenCount==0){
        ChessHNode_deleteChildren(node);
        *lineoutLength = 0;
        return node->evaluation;
    }
    //if maximizing player
    int i, eval, best=0;
    ChessHNode* child;
    move_t lines[node->childrenCount][depth+quiecense+deepQuiecense];
    int lineLengths[node->childrenCount];
    if(node->toPlay==WHITE){
        for(i=node->childrenCount-1; i>=0; i--){
            child = node->children[i];
            ChessBoard_makeMove(board, child->move);
            eval = alphabeta(self, child, depth-1, quiecense, deepQuiecense, 
                alpha, beta, lines[i]+1, lineLengths+i);
            ChessBoard_unmakeMove(board);
            if(eval>alpha){
                alpha = eval;
                best = i;
            }
            if(beta < alpha)
                break;
        }
        lineout[0] = node->children[best]->move;
        *lineoutLength = lineLengths[best]+1;
        for(i=1; i < *lineoutLength; i++)
            lineout[i] = lines[best][i];
        ChessHNode_deleteChildren(node);
        return alpha;
    }
    //minimizing player
    else{
        for(i=0; i<node->childrenCount; i++){
            child = node->children[i];
            ChessBoard_makeMove(board, child->move);
            eval = alphabeta(self, child, depth-1, quiecense, deepQuiecense, 
                alpha, beta, lines[i]+1, lineLengths+i);
            ChessBoard_unmakeMove(board);
            if(eval<beta){
                beta = eval;
                best = i;
            }
            if(beta < alpha)
                break;
        }
        lineout[0] = node->children[best]->move;
        *lineoutLength = lineLengths[best]+1;
        for(i=1; i < *lineoutLength; i++)
            lineout[i] = lines[best][i];
        ChessHNode_deleteChildren(node);
        return beta;
    }
}

int depthSearch(SearchThread* self, ChessHNode* start,
    int depth, move_t* line, int* lineLength){
    int real_depth = DEPTH_ORDERS[self->searchType][depth];
    int quiecense = QUIECENSE_ORDERS[self->searchType][depth];
    int deep_quiecense = DEEP_QUIECENSE_ORDERS[self->searchType][depth];
    int eval = alphabeta(
        self, start, real_depth, quiecense, deep_quiecense,
        INT_MIN, INT_MAX, line, lineLength
    );
    return eval;
}

void* searchMain(void* args){
    SearchThread* self = (SearchThread*)args;
    ChessHNode* start = ChessHNode_new(NULL, self->board);
    move_t tempLine[MAX_LINE_LENGTH];
    int tempLineLength, eval, depth, i;
    for(depth = 0; depth<MAX_DEPTH && !isDying(self); depth++){
        eval = depthSearch(self, start, depth, line, lineLength);
        if(!isDying(self)){
            ChessMutex_lock(self->bestLineMutex);
            self->bestLineLength = tempLineLength;
            self->bestLineValue = eval;
            for(i=0; i<tempLineLength; i++)
                self->bestLine[i] = tempLine[i];
            ChessMutex_unlock(self->bestLineMutex);
        }
    }
}
