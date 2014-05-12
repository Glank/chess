#include "pgn.h"
#include <stdlib.h>

typedef struct PGNNode PGNNode;

struct PGNRecord{
    char* source;
    int sourceLength;
    result_e result;
    PGNNode* movesHead;
    PGNNode* movesTail;
};

struct PGNNode{
    move_t move;
    PGNNode* next;
};
PGNNode* PGNNode_new(move_t move, PGNNode* next){
    PGNNode* self = (PGNNode*)
        malloc(sizeof(PGNNode));
    self->move = move;
    self->next = next;
    return self;
}

PGNRecord* PGNRecord_newFromBoard(ChessBoard* board, 
    int drawAssumable){
    PGNRecord* self = (PGNRecord*)
        malloc(sizeof(PGNRecord));
    self->source = NULL;
    self->sourceLength = -1;
    //derive the game result
    ChessMoveGenerator* gen = ChessMoveGenerator_new(board);
    int inCheck = ChessBoard_testForCheck(board);
    ChessMoveGenerator_generateMoves(gen, inCheck, NULL);
    if(gen->nextCount==0){
        if(inCheck){
            if(gen->toPlay==WHITE)
                self->result=BLACK_VICTORY;
            else
                self->result=WHITE_VICTORY;
        }
        else
            self->result=DRAW;
    }
    else if(drawAssumable && ChessBoard_isInOptionalDraw(board))
        self->result=DRAW;
    else
        self->result=OTHER_RESULT;

    GameInfo* info = (GameInfo*)board->extra;
    if(info->movesCount==0){
        self->movesHead = NULL;
        self->movesTail = NULL;
    }
    else{
        //build the move list tail first
        PGNNode* cur = 
            PGNNode_new(info->moves[info->movesCount-1], NULL);    
        self->movesTail = cur;
        int i;
        for(i = info->movesCount-2; i>=0; i--)
            cur = PGNNode_new(info->moves[i], cur);
        self->movesHead = cur;
    }
    return self;
}
