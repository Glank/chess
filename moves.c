#include "moves.h"

void __initIterValues(ChessMoveGenerator* self);
void __generatePawnMoves(ChessMoveGenerator* self);
void __generateBishopMoves(ChessMoveGenerator* self);
void __generateKnightMoves(ChessMoveGenerator* self);
void __generateRookMoves(ChessMoveGenerator* self);
void __generateQueenMoves(ChessMoveGenerator* self);
void __generateKingMoves(ChessMoveGenerator* self);
void __generateCastlings(ChessMoveGenerator* self);
void __generateEnPassant(ChessMoveGenerator* self);
void __testForCheck(ChessBoard* board, color_e color);

ChessMoveGenerator* ChessMoveGenerator_new(){
    ChessMoveGenerator* self = (ChessMoveGenerator*)
        malloc(sizeof(ChessMoveGenerator));
    self->tempNext = (ChessBoard**)malloc(
        sizeof(ChessBoard*)*MOVE_GEN_MAX_ALLOCATED);
    self->tempNextFilled = 0;
    self->currentSet = (ChessBoard**)malloc(
        sizeof(ChessBoard*)*MOVE_GEN_MAX_ALLOCATED);
    return self;
}
void ChessMoveGenerator_delete(ChessMoveGenerator* self){
    free(self->tempNext);
    free(self);
}
void ChessMoveGenerator_generateMoves(
    ChessMoveGenerator* self, ChessBoard* from){
    self->currentBoard = from;
    __initIterValues(self);
    __generatePawnMoves(self);
    __generateBishopMoves(self);
    __generateKnightMoves(self);
    __generateRookMoves(self);
    __generateQueenMoves(self);
    __generateKingMoves(self);
    __generateCastlings(self);
    __generateEnPassant(self);
}

void __initIterValues(ChessMoveGenerator* self){
    self->tempNextFilled = 0;
    flag_t flags = self->currentBoard->flags;
    self->toPlay = flags&TO_PLAY_FLAG?BLACK:WHITE;
    if(self->toPlay==WHITE){
        self->inCheck = flags&WHITE_IN_CHECK;
        self->curSet = self->currentBoard->whitePieces;
        self->othSet = self->currentBoard->blackPieces;
    }
    else{
        self->inCheck = flags&BLACK_IN_CHECK;
        self->curSet = self->currentBoard->blackPieces;
        self->othSet = self->currentBoard->whitePieces;
    }
}

void __testForCheck(ChessBoard* board, color_e color){
    ChessPieceSet* pieces = color==WHITE?
        board->whitePieces:board->blackPieces;
    ChessPieceSet* opPieces = color==WHITE?
        board->blackPieces:board->whitePieces;
    ChessPiece* king = pieces->piecesByType[0][0];
    int kingRank, kingFile;
    kingRank = GET_RANK(king->location);
    kingFile = GET_FILE(king->location);
    //test for check by a pawn
    if((color==WHITE && kingRank<6) || (color==BLACK && kingRank>1)){
        int opPawnDirection = color==WHITE?-1:1;
        int pawnRank = kingRank-opPawnDirection;
        ChessPiece* pawn;
        if(kingFile>0){
            pawn = board->squares[RANK_FILE(pawnRank, kingFile-1)];
            if((pawn!=NULL) && (pawn->type==PAWN) && 
                (pawn->color!=color))
                return 1;
        }
        if(kingFile<7){
            pawn = board->squares[RANK_FILE(pawnRank, kingFile+1)];
            if((pawn!=NULL) && (pawn->type==PAWN) && 
                (pawn->color!=color))
                return 1;
        }
    }
    //test for check by knight
    int knightRank, knightFile;
    ChessPiece* knight;
    if(kingRank>1){
        knightRank = kingRank-2;
        if(kingFile>0){
            knightFile = kingFile-1;
            knight = board->squares[RANK_FILE(knightRank, knightFile)];
            if((knight!=NULL) && (knight->type==KNIGHT) && 
                (knight->color!=color))
                return 1;
        }
        if(kingFile<7){
            knightFile = kingFile+1;
            knight = board->squares[RANK_FILE(knightRank, knightFile)];
            if((knight!=NULL) && (knight->type==KNIGHT) && 
                (knight->color!=color))
                return 1;
        }
    }
    if(kingRank<6){
        knightRank = kingRank+2;
        if(kingFile>0){
            knightFile = kingFile-1;
            knight = board->squares[RANK_FILE(knightRank, knightFile)];
            if((knight!=NULL) && (knight->type==KNIGHT) && 
                (knight->color!=color))
                return 1;
        }
        if(kingFile<7){
            knightFile = kingFile+1;
            knight = board->squares[RANK_FILE(knightRank, knightFile)];
            if((knight!=NULL) && (knight->type==KNIGHT) && 
                (knight->color!=color))
                return 1;
        }
    }
    if(kingFile>1){
        knightFile = kingFile-2;
        if(kingRank>0){
            knightRank = kingRank-1;
            knight = board->squares[RANK_FILE(knightRank, knightFile)];
            if((knight!=NULL) && (knight->type==KNIGHT) && 
                (knight->color!=color))
                return 1;
        }
        if(kingRank<7){
            knightRank = kingRank+1;
            knight = board->squares[RANK_FILE(knightRank, knightFile)];
            if((knight!=NULL) && (knight->type==KNIGHT) && 
                (knight->color!=color))
                return 1;
        }
    }
    if(kingFile<6){
        knightFile = kingFile+2;
        if(kingRank>0){
            knightRank = kingRank-1;
            knight = board->squares[RANK_FILE(knightRank, knightFile)];
            if((knight!=NULL) && (knight->type==KNIGHT) && 
                (knight->color!=color))
                return 1;
        }
        if(kingRank<7){
            knightRank = kingRank+1;
            knight = board->squares[RANK_FILE(knightRank, knightFile)];
            if((knight!=NULL) && (knight->type==KNIGHT) && 
                (knight->color!=color))
                return 1;
        }
    }
    //test for check by queen
}
