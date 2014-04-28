#include "heuristics.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <limits.h>
#include <time.h>
#define PAWN_VALUE      100
#define KNIGHT_VALUE    300
#define BISHOP_VALUE    300
#define ROOK_VALUE      500
#define QUEEN_VALUE     1000
#define MOBILITY_VALUE  10

ChessMoveGenerator* __gen; //a private generator just for evaluations

void initChessHeuristics(ChessBoard* board){
    srand(time(NULL));
    __gen = ChessMoveGenerator_new(board);
}

void closeChessHeuristics(){
    ChessMoveGenerator_delete(__gen);
}

int ChessBoard_isInOptionalDraw(ChessBoard* board){
    if(board->fiftyMoveCount>=50)
        return 1;
    int i;
    int count=1;
    for(i = 0; i < board->fiftyMoveCount; i++){
        if(board->backups[i].hash==board->hash)
            count++;
    }
    //this is slopy and will have some errors, but it's way quicker than
    //a perfect check
    if(count>=3)
        return 1;
    return 0;
}

ChessHNode* ChessHNode_new(ChessHNode* parent, ChessBoard* board){
    ChessHNode* self = (ChessHNode*)malloc(
        sizeof(ChessHNode));
    self->parent = parent;
    self->children = NULL;
    self->childrenCount = -1;
    if(parent!=NULL)
        self->move = board->moves[board->movesCount-1];
    else
        self->move = 0;
    self->toPlay = board->flags&TO_PLAY_FLAG?BLACK:WHITE;
    self->inCheck = ChessBoard_testForCheck(board);
    self->hash = board->hash;
    self->halfMoveNumber = board->movesCount;
    if(parent!=NULL)
        self->depth = parent->depth+1;
    else
        self->depth = 0; //is root

    self->evaluation = 0;
    self->state = UN_EVAL;
    self->type = ESTIMATE;
    return self;
}
void ChessHNode_delete(ChessHNode* self){
    ChessHNode_deleteChildren(self);
    free(self);
}
void ChessHNode_deleteChildren(ChessHNode* self){
    if(self->children!=NULL){
        int i;
        for(i = 0; i < self->childrenCount; i++)
            ChessHNode_delete(self->children[i]);
        free(self->children);
        self->children = NULL;
    }
}
void ChessHNode_doPreEvaluation(ChessHNode* self, ChessBoard* board){
    assert(self->state==UN_EVAL);
    if(ChessBoard_isInOptionalDraw(board)){
        self->evaluation = 0;
        self->state = FULL_EVAL;
        self->type = ESTIMATE;
        return;
    }
    int eval = 0;
    //just add and subtract the values of each piece
    eval+=board->pieceSets[WHITE]->piecesCounts[PAWN_INDEX]*PAWN_VALUE;
    eval+=board->pieceSets[WHITE]->piecesCounts[KNIGHT_INDEX]*KNIGHT_VALUE;
    eval+=board->pieceSets[WHITE]->piecesCounts[BISHOP_INDEX]*BISHOP_VALUE;
    eval+=board->pieceSets[WHITE]->piecesCounts[ROOK_INDEX]*ROOK_VALUE;
    eval+=board->pieceSets[WHITE]->piecesCounts[QUEEN_INDEX]*QUEEN_VALUE;

    eval-=board->pieceSets[BLACK]->piecesCounts[PAWN_INDEX]*PAWN_VALUE;
    eval-=board->pieceSets[BLACK]->piecesCounts[KNIGHT_INDEX]*KNIGHT_VALUE;
    eval-=board->pieceSets[BLACK]->piecesCounts[BISHOP_INDEX]*BISHOP_VALUE;
    eval-=board->pieceSets[BLACK]->piecesCounts[ROOK_INDEX]*ROOK_VALUE;
    eval-=board->pieceSets[BLACK]->piecesCounts[QUEEN_INDEX]*QUEEN_VALUE;

    //fuzz
    eval+=(rand()%11)-5;

    self->evaluation = eval;
    self->state = PRE_EVAL;
    self->type = ESTIMATE;
}

//called if a node has no children
void __judgeTeminal(ChessHNode* self){
    self->state = FULL_EVAL;
    self->type = ABSOLUTE;
    if(self->inCheck){
        if(self->toPlay==WHITE)
            self->evaluation = INT_MIN;
        else
            self->evaluation = INT_MAX;
    }
    else
        self->evaluation = 0;
}

//slow, should only be done on leaf nodes
void ChessHNode_doFullEvaluation(ChessHNode* self, ChessBoard* board){
    if(self->state == FULL_EVAL)
        return;
    if(self->state==UN_EVAL)
        ChessHNode_doPreEvaluation(self, board);
    ChessMoveGenerator_generateMoves(__gen, self->inCheck, NULL);
    if(__gen->nextCount==0){
        __judgeTeminal(self);
        return;
    }
    self->state = FULL_EVAL;
    //approximate mobility of the last player by their mobility last move
    int whiteMobility, blackMobility;
    if(self->toPlay==WHITE){
        whiteMobility = __gen->nextCount;
        blackMobility = self->parent==NULL?0:self->parent->childrenCount;
    }
    else{
        blackMobility = __gen->nextCount;
        whiteMobility = self->parent==NULL?0:self->parent->childrenCount;
    }
    self->evaluation+=whiteMobility;
    self->evaluation-=blackMobility;
}

ChessHNode* __tempChildren[MOVE_GEN_MAX_ALLOCATED];
int __tempChildrenCount;
ChessHNode* __tempParent;
void __pushAndPreEvalNewTempChild(ChessBoard* board){
    ChessHNode* child = ChessHNode_new(__tempParent, board);
    ChessHNode_doPreEvaluation(child, board);
    //sorted insertion
    int i;
    for(i = __tempChildrenCount-1; i>=0; i--){
        if(__tempChildren[i]->evaluation<=child->evaluation)
            break;
        else
            __tempChildren[i+1] = __tempChildren[i];
    }
    __tempChildren[i+1] = child;
    __tempChildrenCount++;
}
void __pushAndFullEvalNewTempChild(ChessBoard* board){
    ChessHNode* child = ChessHNode_new(__tempParent, board);
    ChessHNode_doFullEvaluation(child, board);
    //unsorted push
    __tempChildren[__tempChildrenCount++] = child;
}

void ChessHNode_expandBranches(ChessHNode* self, ChessMoveGenerator* gen){
    assert(self->children==NULL);
    __tempChildrenCount = 0;
    __tempParent = self;
    ChessMoveGenerator_generateMoves(gen, self->inCheck, &__pushAndPreEvalNewTempChild);
    self->childrenCount = __tempChildrenCount;
    if(__tempChildrenCount==0){
        __judgeTeminal(self);
        return;
    }
    self->children = (ChessHNode**)malloc(sizeof(ChessHNode*)*__tempChildrenCount);
    int i;
    for(i=0; i<__tempChildrenCount; i++)
        self->children[i] = __tempChildren[i];
}

void ChessHNode_expandLeaves(ChessHNode* self, ChessMoveGenerator* gen){
    assert(self->children==NULL);
    __tempChildrenCount = 0;
    __tempParent = self;
    ChessMoveGenerator_generateMoves(gen, self->inCheck, &__pushAndFullEvalNewTempChild);
    self->childrenCount = __tempChildrenCount;
    if(__tempChildrenCount==0){
        __judgeTeminal(self);
        return;
    }
    self->children = (ChessHNode**)malloc((sizeof(ChessHNode*)*__tempChildrenCount));
    int i;
    for(i=0; i<__tempChildrenCount; i++)
        self->children[i] = __tempChildren[i];
}
