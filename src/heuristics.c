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
#define PAWN_PUSH_END_GAME 4
#define PAWN_CONNECTION_VALUE 2
#define OPENED_PEICES_COST 4
#define KNIGHTS_ON_THE_RIM_COST 4
#define WN1 1
#define WN2 6
#define WB1 2
#define WB2 5
#define WQ 3
#define BN1 62
#define BB1 61
#define BQ 59
#define BB2 58
#define BN2 57

ChessHEngine* ChessHEngine_new(ChessBoard* board){
    srand(time(NULL));
    ChessHEngine* self = (ChessHEngine*)malloc(sizeof(ChessHEngine));
    self->board = board;
    self->expGen = ChessMoveGenerator_new(board);
    self->evalGen = ChessMoveGenerator_new(board);
    return self;
}
void ChessHEngine_delete(ChessHEngine* self){
    ChessMoveGenerator_delete(self->expGen);
    ChessMoveGenerator_delete(self->evalGen);
    free(self);
}

ChessHNode* ChessHNode_new(ChessHNode* parent, ChessHEngine* engine){
    ChessBoard* board = engine->board;
    GameInfo* gameInfo = (GameInfo*)board->extra;

    ChessHNode* self = (ChessHNode*)malloc(sizeof(ChessHNode));
    self->parent = parent;
    self->children = NULL;
    self->childrenCount = -1;
    if(parent!=NULL)
        self->move = gameInfo->moves[gameInfo->movesCount-1];
    else
        self->move = 0;
    self->toPlay = board->flags&TO_PLAY_FLAG?BLACK:WHITE;
    self->inCheck = ChessBoard_testForCheck(board);
    self->hash = board->hash;
    self->halfMoveNumber = gameInfo->movesCount;
    if(parent!=NULL)
        self->depth = parent->depth+1;
    else
        self->depth = 0; //is root

    self->evaluation = 0;
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
void ChessHNode_evaluate(ChessHNode* self, ChessHEngine* engine){
    ChessBoard* board = engine->board;
    if(ChessBoard_isInOptionalDraw(board)){
        self->evaluation = 0;
        self->type = ABSOLUTE;
        return;
    }
    int eval = 0;
    //just add and subtract the values of each piece
    GameInfo* info = (GameInfo*)board->extra;
    
    int white_pieces_value = 0;
    white_pieces_value+=info->pieceSets[WHITE]->piecesCounts[PAWN_INDEX]*PAWN_VALUE;
    white_pieces_value+=info->pieceSets[WHITE]->piecesCounts[KNIGHT_INDEX]*KNIGHT_VALUE;
    white_pieces_value+=info->pieceSets[WHITE]->piecesCounts[BISHOP_INDEX]*BISHOP_VALUE;
    white_pieces_value+=info->pieceSets[WHITE]->piecesCounts[ROOK_INDEX]*ROOK_VALUE;
    white_pieces_value+=info->pieceSets[WHITE]->piecesCounts[QUEEN_INDEX]*QUEEN_VALUE;

    int black_pieces_value = 0;
    black_pieces_value+=info->pieceSets[BLACK]->piecesCounts[PAWN_INDEX]*PAWN_VALUE;
    black_pieces_value+=info->pieceSets[BLACK]->piecesCounts[KNIGHT_INDEX]*KNIGHT_VALUE;
    black_pieces_value+=info->pieceSets[BLACK]->piecesCounts[BISHOP_INDEX]*BISHOP_VALUE;
    black_pieces_value+=info->pieceSets[BLACK]->piecesCounts[ROOK_INDEX]*ROOK_VALUE;
    black_pieces_value+=info->pieceSets[BLACK]->piecesCounts[QUEEN_INDEX]*QUEEN_VALUE;
    
    eval+=white_pieces_value;
    eval-=black_pieces_value;

    //pushing and connecting pawns is important, especially in the end game
    {
        int i;
        ChessPiece* pawn, * connection;
        int size, push, rank, file;
        //white pawns
        size = info->pieceSets[WHITE]->piecesCounts[PAWN_INDEX];
        for(i=0; i < size; i++){
            pawn = info->pieceSets[WHITE]->piecesByType[PAWN_INDEX][i];
            rank = GET_RANK(pawn->location);
            if(self->halfMoveNumber>50){ //if end game
                push = rank-2;
                eval+= push*push*PAWN_PUSH_END_GAME; //push
            }
            else{
                file = GET_FILE(pawn->location);
                if(rank<6){ //connections
                    if(file>0){
                        connection = board->squares[RANK_FILE(rank+1,file-1)];
                        if(connection!=NULL && connection->type==PAWN && connection->color==WHITE)
                            eval+=PAWN_CONNECTION_VALUE;
                    }
                    if(file<7){
                        connection = board->squares[RANK_FILE(rank+1,file+1)];
                        if(connection!=NULL && connection->type==PAWN && connection->color==WHITE)
                            eval+=PAWN_CONNECTION_VALUE;
                    }
                }
            }
        }
        //black pawns
        size = info->pieceSets[BLACK]->piecesCounts[PAWN_INDEX];
        for(i=0; i < size; i++){
            pawn = info->pieceSets[BLACK]->piecesByType[PAWN_INDEX][i];
            rank = GET_RANK(pawn->location);
            if(self->halfMoveNumber>50){ //if end game
                push = 6-rank;
                eval-= push*push*PAWN_PUSH_END_GAME; //push
            }
            else{
                file = GET_FILE(pawn->location);
                if(rank>1){ //connections
                    if(file>0){
                        connection = board->squares[RANK_FILE(rank-1,file-1)];
                        if(connection!=NULL && connection->type==PAWN && connection->color==BLACK)
                            eval-=PAWN_CONNECTION_VALUE;
                    }
                    if(file<7){
                        connection = board->squares[RANK_FILE(rank-1,file+1)];
                        if(connection!=NULL && connection->type==PAWN && connection->color==BLACK)
                            eval-=PAWN_CONNECTION_VALUE;
                    }
                }
            }
        }
    }

    //opening heuristics
    if(self->halfMoveNumber<=20){
        ChessPiece* piece;
        //white
        piece = board->squares[WN1];
        if(piece!=NULL && piece->type==KNIGHT && piece->color==WHITE)
            eval-=OPENED_PEICES_COST;
        piece = board->squares[WN2];
        if(piece!=NULL && piece->type==KNIGHT && piece->color==WHITE)
            eval-=OPENED_PEICES_COST;
        piece = board->squares[WB1];
        if(piece!=NULL && piece->type==BISHOP && piece->color==WHITE)
            eval-=OPENED_PEICES_COST;
        piece = board->squares[WB2];
        if(piece!=NULL && piece->type==BISHOP && piece->color==WHITE)
            eval-=OPENED_PEICES_COST;
        piece = board->squares[WQ];
        if(piece!=NULL && piece->type==QUEEN && piece->color==WHITE)
            eval-=OPENED_PEICES_COST;
        //black
        piece = board->squares[BN1];
        if(piece!=NULL && piece->type==KNIGHT && piece->color==BLACK)
            eval+=OPENED_PEICES_COST;
        piece = board->squares[BN2];
        if(piece!=NULL && piece->type==KNIGHT && piece->color==BLACK)
            eval+=OPENED_PEICES_COST;
        piece = board->squares[BB1];
        if(piece!=NULL && piece->type==BISHOP && piece->color==BLACK)
            eval+=OPENED_PEICES_COST;
        piece = board->squares[BB2];
        if(piece!=NULL && piece->type==BISHOP && piece->color==BLACK)
            eval+=OPENED_PEICES_COST;
        piece = board->squares[BQ];
        if(piece!=NULL && piece->type==QUEEN && piece->color==BLACK)
            eval+=OPENED_PEICES_COST;
    }

    //knights on the rim
    {
        ChessPiece* knight;
        int i, size, rank, file;
        //white
        size = info->pieceSets[WHITE]->piecesCounts[KNIGHT_INDEX];
        for(i=0; i < size; i++){
            knight = info->pieceSets[WHITE]->piecesByType[KNIGHT_INDEX][i];
            rank = GET_RANK(knight->location);
            if(rank==0 || rank==7){
                eval-=KNIGHTS_ON_THE_RIM_COST;            
                continue;
            }
            file = GET_FILE(knight->location);
            if(file==0 || file==7){
                eval-=KNIGHTS_ON_THE_RIM_COST;            
                continue;
            }
        }
        //black
        size = info->pieceSets[BLACK]->piecesCounts[KNIGHT_INDEX];
        for(i=0; i < size; i++){
            knight = info->pieceSets[BLACK]->piecesByType[KNIGHT_INDEX][i];
            rank = GET_RANK(knight->location);
            if(rank==0 || rank==7){
                eval+=KNIGHTS_ON_THE_RIM_COST;            
                continue;
            }
            file = GET_FILE(knight->location);
            if(file==0 || file==7){
                eval+=KNIGHTS_ON_THE_RIM_COST;            
                continue;
            }
        }
    }

    //mop up heuristic
    {
        ChessPiece* king, * op_king;
        int rank, file;
        //cmd = op kings center manhatan distance
        //md = manhatin distance between the two kings
        int add_mopup=0, favor, cmd, md, rank_delta, file_delta;
        //if white is winning a king-rook v king type situation
        if(black_pieces_value<ROOK_VALUE && white_pieces_value>=ROOK_VALUE){
            king = info->pieceSets[WHITE]->piecesByType[KING_INDEX][0];
            op_king = info->pieceSets[BLACK]->piecesByType[KING_INDEX][0];
            add_mopup=1;
            favor=10;
        }//if black is winning...
        else if(white_pieces_value<ROOK_VALUE && black_pieces_value>=ROOK_VALUE){
            king = info->pieceSets[WHITE]->piecesByType[KING_INDEX][0];
            op_king = info->pieceSets[BLACK]->piecesByType[KING_INDEX][0];
            add_mopup=1;
            favor=-10;
        }
        if(add_mopup){
            rank = GET_RANK(op_king->location);
            file = GET_FILE(op_king->location);
            rank_delta = rank*2-7;
            rank_delta = rank_delta<0?-rank_delta:rank_delta; // abs(rank_delta)
            file_delta = file*2-7;
            file_delta = file_delta<0?-file_delta:file_delta; // abs(file_delta)
            cmd = rank_delta+file_delta;
            rank_delta = rank-GET_RANK(king->location);
            rank_delta = rank_delta<0?-rank_delta:rank_delta; // abs(rank_delta)
            file_delta = file-GET_FILE(king->location);
            file_delta = file_delta<0?-file_delta:file_delta; // abs(file_delta)
            md = rank_delta+file_delta;
            eval+=favor*((2*cmd-md)*8-self->halfMoveNumber);
        }
    }

    //fuzz
    eval+=(rand()%11)-5;

    self->evaluation = eval;
    self->type = ESTIMATE;
}

//called if a node has no children
void __judgeTeminal(ChessHNode* self){
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
ChessHNode* __tempChildren[MOVE_GEN_MAX_ALLOCATED];
int __tempChildrenCount;
ChessHNode* __tempParent;
ChessHEngine* __tempEngine;
void __pushAndEvalNewTempChild(ChessBoard* board){
    ChessHNode* child = ChessHNode_new(__tempParent, __tempEngine);
    ChessHNode_evaluate(child, __tempEngine);
    __tempChildren[__tempChildrenCount++] = child;
}

void ChessHNode_expand(ChessHNode* self, ChessHEngine* engine){
    ChessMoveGenerator* gen = engine->expGen;
    assert(self->children==NULL);
    __tempChildrenCount = 0;
    __tempParent = self;
    __tempEngine = engine;
    ChessMoveGenerator_generateMoves(gen, self->inCheck, &__pushAndEvalNewTempChild);
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

