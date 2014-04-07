#include "heuristics.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#define PAWN_VALUE      100
#define KNIGHT_VALUE    300
#define BISHOP_VALUE    300
#define ROOK_VALUE      500
#define QUEEN_VALUE     1000

ChessMoveGenerator* __gen; //a private generator just for evaluations

void initChessHeuristics(ChessBoard* board){
    __gen = ChessMoveGenerator_new(board);
}

void closeChessHeuristics(){
    ChessMoveGenerator_delete(__gen);
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
    if(self->children!=NULL){
        int i;
        for(i = 0; i < self->childrenCount; i++)
            ChessHNode_delete(self->children[i]);
        free(self->children);
    }
    free(self);
}
void ChessHNode_doPreEvaluation(ChessHNode* self, ChessBoard* board){
    assert(self->state==UN_EVAL);
    int eval = 0;
    int i;
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

    self->evaluation = eval;
    self->state = PRE_EVAL;
    self->type = ESTIMATE;
}

void ChessHNode_doFullEvaluation(ChessHNode* self, ChessBoard* board){
    assert(self->state != FULL_EVAL)
    if(self->state==UN_EVAL)
        ChessHNode_doPreEvaluation(self, board);
    //TODO test for checkmate, stalemate
    self->state = FULL_EVAL;
}

ChessHNode* __tempChildren[MOVE_GEN_MAX_ALLOCATED];
int __tempChildrenCount;
ChessBoard* __tempParent;
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

void ChessHNode_expandBranches(ChessHNode* self, ChessMoveGenerator* gen){
    assert(self->children==NULL);
    __tempChildrenCount = 0;
    ChessMoveGenerator_generateMoves(gen, self->inCheck, &__pushAndPreEvalNewTempChild);
    self->children = (ChessHNode**)malloc(sizeof(ChessHNode*)*__tempChildrenCount);
    int i;
    for(i=0; i<__tempChildrenCount; i++);
        self->children[i] = __tempChildren[i];
    self->childrenCount = __tempChildrenCount;
}

void ChessHNode_expandLeaves(ChessHNode* self, ChessMoveGenerator* gen){
}
