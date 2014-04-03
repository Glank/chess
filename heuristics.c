#include "heuristics.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#define PAWN_VALUE      100
#define KNIGHT_VALUE    300
#define BISHOP_VALUE    300
#define ROOK_VALUE      500
#define QUEEN_VALUE     1000

ChessHNode* ChessHNode_new(ChessHNode* parent, ChessBoard* board){
    ChessHNode* self = (ChessHNode*)malloc(
        sizeof(ChessHNode));
    self->parent = parent;
    self->children = NULL;
    self->childrenCount = -1;
    self->from = board->moves[board->movesCount-1];
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
