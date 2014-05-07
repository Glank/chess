#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <limits.h>
#include "search.h"
#include "narrator.h"
#define MAX_DEPTH 8

TTable* TTable_new(){
    TTable* self = (TTable*)malloc(sizeof(TTable));
    self->minHalfMoveNumber = 0;
    int i;
    for(i = 0; i < TTABLE_SIZE; i++)
        self->nodes[i].hash = 0;
    return self;
}
void TTable_delete(TTable* self){
    free(self);
}
TNode* TTable_lookup(TTable* self, ChessHNode* state){
    int bucket = state->hash%TTABLE_SIZE;
    TNode* ret = self->nodes+bucket;
    if(ret->hash==state->hash)
        return ret;
    return NULL;
}
void TTable_trySave(TTable* self, TNode* node){
    int bucket = node->hash%TTABLE_SIZE;
    TNode* cur = self->nodes+bucket;
    if(cur->hash==0 || cur->halfMoveNumber<self->minHalfMoveNumber){
        (*cur) = (*node);
        return;
    }
    if(cur->isCut){
        if(node->isCut && node->depth <= cur->depth)
            (*cur) = (*node);
        return;
    }
    if(node->isCut || node->depth <= cur->depth)
        (*cur) = (*node);
}
int TTable_isInverted(TTable* self, int minimizing, 
    ChessHNode* a, TNode* a_node, ChessHNode* b, TNode* b_node){
    if(a_node==NULL){
        if(b_node==NULL){
            if(minimizing)
                return a->evaluation > b->evaluation;
            else
                return a->evaluation < b->evaluation;
        }
        else{
            if(b_node->isCut)
                return 1;
            else if(minimizing)
                return a->evaluation > b_node->evaluation;
            else
                return a->evaluation < b_node->evaluation;
        }
    }
    else{
        if(b_node==NULL){
            if(a_node->isCut)
                return 0;
            else if(minimizing)
                return a_node->evaluation > b->evaluation;
            else
                return a_node->evaluation < b->evaluation;
        }
        if(a_node->isCut && !b_node->isCut)
            return 0;
        else if(!a_node->isCut && b_node->isCut)
            return 1;
        else if(minimizing)
            return a_node->evaluation > b_node->evaluation;
        else
            return a_node->evaluation < b_node->evaluation;
    }
}
void TTable_recursiveMergeSort(TTable* self, ChessHNode* toSort, int offset, int length){
    if(length==1)
        return;
    else if(length==2){
        if(TTable_isInverted(self, toSort->toPlay,
            toSort->children[offset],self->sortingRoom[offset],
            toSort->children[offset+1],self->sortingRoom[offset+1])){
            //invert
            TNode* ttemp = self->sortingRoom[offset];
            ChessHNode* htemp = toSort->children[offset];
            self->sortingRoom[offset] = self->sortingRoom[offset+1];
            toSort->children[offset] = toSort->children[offset+1];
            self->sortingRoom[offset+1] = ttemp;
            toSort->children[offset+1] = htemp;
        }
    }
    else{
        int l_off = offset;
        int r_off = offset+length/2;
        int l_len = r_off-l_off;
        int r_len = length/2;
        TTable_recursiveMergeSort(self, toSort, l_off, l_len);
        TTable_recursiveMergeSort(self, toSort, r_off, r_len);
        //merge
        int i;
        for(i=0; i<length; i++){
            if(l_len==0 || (r_len!=0 && TTable_isInverted(self, toSort->toPlay,
                toSort->children[l_off],self->sortingRoom[l_off],
                toSort->children[r_off],self->sortingRoom[r_off]))){
                self->tMergeRoom[i] = self->sortingRoom[r_off];
                self->hMergeRoom[i] = toSort->children[r_off];
                r_off++;
                r_len--;
            }
            else{
                self->tMergeRoom[i] = self->sortingRoom[l_off];
                self->hMergeRoom[i] = toSort->children[l_off];
                l_off++;
                l_len--;
            }
        }
        //copy from merge room
        for(i=0; i<length; i++){
            toSort->children[offset+i] = self->hMergeRoom[i];
            self->sortingRoom[offset+i] = self->tMergeRoom[i];
        }
    }
}
void TTable_sortChildren(TTable* self, ChessHNode* toSort){
    int i;
    for(i = 0; i < toSort->childrenCount; i++)
        self->sortingRoom[i] = TTable_lookup(self, toSort->children[i]);
    TTable_recursiveMergeSort(self, toSort, 0, toSort->childrenCount);
}

const int DEPTH_ORDERS[4][9] = {
    {1,2,3,4,5,6,7,8,9}, //opening depths
    {1,2,2,3,3,3,4,4,4}, //midgame depths
    {1,2,2,3,3,4,4,5,5}, //endgame depths
    {1,2,2,3,3,3,4,4,4}  //puzzle  depths
};
const int QUIECENSE_ORDERS[4][9] = {
    {0,0,1,1,2,2,3,3,4}, //opening depths
    {0,0,1,1,2,2,3,3,4}, //midgame depths
    {0,0,1,1,2,2,3,3,4}, //endgame depths
    {0,0,1,1,2,2,3,3,4}  //puzzle  depths
};
const int DEEP_QUIECENSE_ORDERS[4][9] = {
    {0,0,1,1,2,2,3,3,4}, //opening depths
    {0,0,1,1,2,2,3,3,4}, //midgame depths
    {0,0,1,1,2,2,3,3,4}, //endgame depths
    {0,0,1,1,2,2,3,3,4}  //puzzle  depths
};

void* searchMain(void* args);
int alphabeta(
    SearchThread* self, ChessHNode* node, 
    int depth, int quiecense, int deepQuiecense,
    int alpha, int beta,
    move_t* lineout, int* lineoutLength);
int isDying(SearchThread* self);
SearchThread* SearchThread_new(ChessBoard* board, TTable* table){
    SearchThread* self = (SearchThread*)malloc(sizeof(SearchThread));
    self->thread = ChessThread_new(&searchMain);
    self->bestLineMutex = ChessMutex_new();
    self->max_seconds = 60L;

    self->board = board;
    self->searchType = PUZZLE;
    self->engine = ChessHEngine_new(board);
    self->table = table;

    self->bestLineLength = 0;
    self->bestLineValue = 0;
    self->printEachNewLine = 0;
    return self;
}
void SearchThread_delete(SearchThread* self){
    ChessHEngine_delete(self->engine);
    ChessThread_delete(self->thread);
    ChessMutex_delete(self->bestLineMutex);
    free(self);
}
void SearchThread_setSearchType(SearchThread* self, searchType_e type){
    self->searchType = type;
}
searchType_e SearchThread_getSearchType(SearchThread* self){
    return self->searchType;
}
void SearchThread_setTimeout(SearchThread* self, time_t max_seconds){
    self->max_seconds = max_seconds;
}
long SearchThread_getTimeout(SearchThread* self){
    return self->max_seconds;
}
int SearchThread_getBestLine(SearchThread* self, move_t* lineOut, int* lineLength){
    int ret;
    ChessMutex_lock(self->bestLineMutex);
        int i;
        for(i=0; i<self->bestLineLength; i++)
            lineOut[i] = self->bestLine[i];
        (*lineLength) = self->bestLineLength;
        ret = self->bestLineValue;
    ChessMutex_unlock(self->bestLineMutex);
    return ret;
}
void SearchThread_start(SearchThread* self){
    self->runFlag = 1;
    self->start_time = time(NULL);
    ChessThread_start(self->thread, self);
}
void SearchThread_stop(SearchThread* self){
    self->runFlag = 0;
    ChessThread_join(self->thread, NULL);
}
void SearchThread_join(SearchThread* self){
    ChessThread_join(self->thread, NULL);
}
int SearchThread_isRunning(SearchThread* self){
    return ChessThread_getState(self->thread) == RUNNING_THREAD;
}
void SearchThread_setPrintEachNewLine(SearchThread* self, int b){
    ChessMutex_lock(self->bestLineMutex);
    self->printEachNewLine = b;
    ChessMutex_unlock(self->bestLineMutex);
}
int SearchThread_getPrintEachNewLine(SearchThread* self){
    int ret;
    ChessMutex_lock(self->bestLineMutex);
    ret = self->printEachNewLine;
    ChessMutex_unlock(self->bestLineMutex);
    return ret;
}
void SearchThread_printBestLine(SearchThread* self){
    int i;
    printf("%d\t", self->bestLineValue);
    char moveOut[10];
    int moveOutSize;
    for(i = 0; i < self->bestLineLength; i++){
        if(i!=0)
            printf(" ");
        toAlgebraicNotation(self->bestLine[i], self->board, moveOut, &moveOutSize);
        printf("%s", moveOut);
        ChessBoard_makeMove(self->board, self->bestLine[i]);
    }
    for(i = 0; i < self->bestLineLength; i++)
        ChessBoard_unmakeMove(self->board);
    printf("\n");
}
move_t SearchThread_getBestMove(SearchThread* self){
    move_t ret;
    ChessMutex_lock(self->bestLineMutex);
    ret = self->bestLine[0];
    ChessMutex_unlock(self->bestLineMutex);
    return ret;
}

int isDying(SearchThread* self){
    return ((!self->runFlag)||
        ((self->max_seconds!=0)&&
        (self->max_seconds < time(NULL)-self->start_time)));
}

int alphabeta(
    SearchThread* self, ChessHNode* node, 
    int depth, int quiecense, int deepQuiecense,
    int alpha, int beta,
    move_t* lineout, int* lineoutLength){
    TNode* tnode = TTable_lookup(self->table, node);
    if(tnode!=NULL && tnode->depth>=depth){
        *lineoutLength = 0;
        return tnode->evaluation;
    }
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
    ChessHNode_expand(node, self->engine);
    //if terminal
    if(node->childrenCount==0){
        ChessHNode_deleteChildren(node);
        *lineoutLength = 0;
        return node->evaluation;
    }
    //else, sort
    TTable_sortChildren(self->table, node);

    int i, best=0;
    ChessHNode* child;
    TNode tnew;
    tnew.depth = depth-1;
    tnew.halfMoveNumber = node->halfMoveNumber+1;
    tnew.isCut = 0;
    move_t lines[node->childrenCount][depth+quiecense+deepQuiecense];
    int lineLengths[node->childrenCount];
    //if maximizing player
    if(node->toPlay==WHITE){
        for(i=0; !tnew.isCut && i<node->childrenCount; i++){
            child = node->children[i];
            tnew.hash = child->hash;
            ChessBoard_makeMove(self->board, child->move);
            tnew.evaluation = alphabeta(self, child, depth-1, quiecense, deepQuiecense, 
                alpha, beta, lines[i]+1, lineLengths+i);
            ChessBoard_unmakeMove(self->board);
            if(tnew.evaluation>alpha){
                alpha = tnew.evaluation;
                best = i;
            }
            if(beta < alpha)
                tnew.isCut=1;
            TTable_trySave(self->table, &tnew);
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
        for(i=0; !tnew.isCut && i<node->childrenCount; i++){
            child = node->children[i];
            tnew.hash = child->hash;
            ChessBoard_makeMove(self->board, child->move);
            tnew.evaluation = alphabeta(self, child, depth-1, quiecense, deepQuiecense, 
                alpha, beta, lines[i]+1, lineLengths+i);
            ChessBoard_unmakeMove(self->board);
            if(tnew.evaluation<beta){
                beta = tnew.evaluation;
                best = i;
            }
            if(beta < alpha)
                tnew.isCut=1;
            TTable_trySave(self->table, &tnew);
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
    self->table->minHalfMoveNumber = start->halfMoveNumber;
    int eval = alphabeta(
        self, start, real_depth, quiecense, deep_quiecense,
        INT_MIN, INT_MAX, line, lineLength
    );
    return eval;
}

void* searchMain(void* args){
    SearchThread* self = (SearchThread*)args;
    ChessHNode* start = ChessHNode_new(NULL, self->engine);
    move_t tempLine[MAX_LINE_LENGTH];
    int tempLineLength, eval, depth, i;
    for(depth = 0; depth<MAX_DEPTH && !isDying(self); depth++){
        eval = depthSearch(self, start, depth, tempLine, &tempLineLength);
        if(!isDying(self)){
            ChessMutex_lock(self->bestLineMutex);
            self->bestLineLength = tempLineLength;
            self->bestLineValue = eval;
            for(i=0; i<tempLineLength; i++)
                self->bestLine[i] = tempLine[i];
            if(self->printEachNewLine)
                SearchThread_printBestLine(self);
            ChessMutex_unlock(self->bestLineMutex);
        }
    }
    ChessHNode_delete(start);
    return NULL;
}
