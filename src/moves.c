#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "moves.h"

void __initIterValues(ChessMoveGenerator* self, int inCheck,
    void (*afterGen)(ChessBoard*));
void __generatePawnMoves(ChessMoveGenerator* self);
void __generateDirectionalMoves(ChessMoveGenerator* self,
    ChessPiece* piece, int dRank, int dFile);
void __generateBishopMoves(ChessMoveGenerator* self);
void __generateKnightMoves(ChessMoveGenerator* self);
void __generateRookMoves(ChessMoveGenerator* self);
void __generateQueenMoves(ChessMoveGenerator* self);
void __generateKingMoves(ChessMoveGenerator* self);
void __generateCastlings(ChessMoveGenerator* self);
void __generateEnPassant(ChessMoveGenerator* self);
int __generateSimpleMove(ChessMoveGenerator* self, ChessPiece* piece, 
    int dRank, int dFile, int validate);
int __finalizeOrDelete(ChessMoveGenerator* self, 
    ChessBoard* board, int validate);
int __testForCheck(ChessBoard* board, color_e color);
ChessBoard* __getCleanBoard(ChessMoveGenerator* self);
void __finish(ChessMoveGenerator* self,
    move_t** to, int* toCount);

ChessMoveGenerator* ChessMoveGenerator_new(ChessBoard* board){
    ChessMoveGenerator* self = (ChessMoveGenerator*)
        malloc(sizeof(ChessMoveGenerator));
    self->nextCount = 0;
    self->board = board;
    return self;
}

void ChessMoveGenerator_delete(ChessMoveGenerator* self){
    free(self);
}

void ChessMoveGenerator_generateMoves(
    ChessMoveGenerator* self, int inCheck,
    void (*afterGen)(ChessBoard*)){
    __initIterValues(self, inCheck, afterGen);
    __generatePawnMoves(self);
    __generateBishopMoves(self);
    __generateRookMoves(self);
    __generateQueenMoves(self);
    __generateKnightMoves(self);
    __generateKingMoves(self);
    __generateEnPassant(self);
    __generateCastlings(self);
}

int ChessBoard_testForCheck(ChessBoard* board){
    assert(board!=NULL);
    return __testForCheck(board, 
        ((board->flags)&TO_PLAY_FLAG)?BLACK:WHITE);
}

void ChessMoveGenerator_copyMoves(ChessMoveGenerator* self,
    move_t** to, int* toCount){
    (*toCount) = self->nextCount;
    assert((*to)==NULL);
    int size = sizeof(move_t)*self->nextCount;
    (*to) = (move_t*)malloc(size);
    memcpy(*to, self->next, size);
}

void __initIterValues(ChessMoveGenerator* self, int inCheck,
    void (*afterGen)(ChessBoard*)){
    self->afterGen = afterGen;
    self->nextCount = 0;
    flag_t flags = self->board->flags;
    self->toPlay = flags&TO_PLAY_FLAG?BLACK:WHITE;
    GameInfo* info = (GameInfo*)self->board->extra;
    self->curSet = info->pieceSets[(int)(self->toPlay)];
    assert(self->curSet!=NULL);
    self->inCheck = inCheck; 
}

int __testForCheck(ChessBoard* board, color_e color){
    GameInfo* info = (GameInfo*)board->extra;
    ChessPieceSet* pieces = info->pieceSets[color];
    ChessPieceSet* opPieces = info->pieceSets[OTHER_COLOR(color)];
    ChessPiece* king = pieces->piecesByType[TYPE_TO_INT(KING)][0];
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
                (pawn->color!=color)){
                return 1;
            }
        }
        if(kingFile<7){
            pawn = board->squares[RANK_FILE(pawnRank, kingFile+1)];
            if((pawn!=NULL) && (pawn->type==PAWN) && 
                (pawn->color!=color)){
                return 1;
            }
        }
    }
    //test for check by knight
    int i, rankDelta, fileDelta, rank, file;
    int size = opPieces->piecesCounts[KNIGHT_INDEX];
    ChessPiece* knight;
    for(i=0; i<size; i++){
        knight = opPieces->piecesByType[KNIGHT_INDEX][i];
        rankDelta = kingRank-GET_RANK(knight->location);
        fileDelta = kingFile-GET_FILE(knight->location);
        if((rankDelta*rankDelta+fileDelta*fileDelta)==5)
            return 1;
    }
    //test for check by bishop
    size = opPieces->piecesCounts[BISHOP_INDEX];
    ChessPiece* bishop;
    for(i=0; i<size; i++){
        bishop = opPieces->piecesByType[BISHOP_INDEX][i];
        rankDelta = kingRank-GET_RANK(bishop->location);
        fileDelta = kingFile-GET_FILE(bishop->location);
        if(rankDelta*rankDelta == fileDelta*fileDelta){
            //the king is on a diagonal with this opposing bishop
            //check every square inbetween to see if the bishop 
            //is blocked 
            assert(rankDelta!=0);
            rankDelta = rankDelta>0?1:-1;
            fileDelta = fileDelta>0?1:-1;
            rank = GET_RANK(bishop->location)+rankDelta;
            file = GET_FILE(bishop->location)+fileDelta;
            while(rank!=kingRank){
                if(board->squares[RANK_FILE(rank,file)]!=NULL)
                    break;
                rank+=rankDelta;
                file+=fileDelta;
            }
            if(rank==kingRank)
                return 1;
        }
    }
    //test for check by rook
    size = opPieces->piecesCounts[TYPE_TO_INT(ROOK)];
    ChessPiece* rook;
    for(i=0; i<size; i++){
        rook = opPieces->piecesByType[TYPE_TO_INT(ROOK)][i];
        rankDelta = kingRank-GET_RANK(rook->location);
        fileDelta = kingFile-GET_FILE(rook->location);
        if((rankDelta == 0)||(fileDelta == 0)){
            //the king is on a rank or a file with this opposing rook 
            //check every square inbetween to see if the rook 
            //is blocked 
            if(fileDelta!=0) fileDelta = fileDelta>0?1:-1;
            if(rankDelta!=0) rankDelta = rankDelta>0?1:-1;
            rank = GET_RANK(rook->location)+rankDelta;
            file = GET_FILE(rook->location)+fileDelta;
            while(rank!=kingRank || file!=kingFile){
                if(board->squares[RANK_FILE(rank,file)]!=NULL)
                    break;
                rank+=rankDelta;
                file+=fileDelta;
            }
            if(rank==kingRank && file==kingFile){
                return 1;
            }
        }
    }
    //test for check by queen
    size = opPieces->piecesCounts[TYPE_TO_INT(QUEEN)];
    ChessPiece* queen;
    for(i=0; i<size; i++){
        queen = opPieces->piecesByType[TYPE_TO_INT(QUEEN)][i];
        rankDelta = kingRank-GET_RANK(queen->location);
        fileDelta = kingFile-GET_FILE(queen->location);
        if((rankDelta*rankDelta == fileDelta*fileDelta)||
            (rankDelta == 0)||(fileDelta == 0)){
            //the king is on a diagonal, rank or file with 
            //this opposing queen, check every square
            //inbetween to see if the queen is blocked 
            if(fileDelta!=0) fileDelta = fileDelta>0?1:-1;
            if(rankDelta!=0) rankDelta = rankDelta>0?1:-1;
            rank = GET_RANK(queen->location)+rankDelta;
            file = GET_FILE(queen->location)+fileDelta;
            while(rank!=kingRank || file!=kingFile){
                if(board->squares[RANK_FILE(rank,file)]!=NULL){
                    break;
                }
                rank+=rankDelta;
                file+=fileDelta;
            }
            if(rank==kingRank && file==kingFile){
                return 1;
            }
        }
    }
    //test for check by other king
    //for hypothetical moves that are blocked
    //by the opposing king
    ChessPiece* opKing = opPieces->piecesByType[TYPE_TO_INT(KING)][0];
    rankDelta = kingRank-GET_RANK(opKing->location);
    fileDelta = kingFile-GET_FILE(opKing->location);
    if((rankDelta*rankDelta+fileDelta*fileDelta)<=2){
        return 1;
    }
    //Otherwise, the king is not in check
    return 0;
}
int __finalizeAndUndo(ChessMoveGenerator* self, int validate){
    if(validate && __testForCheck(self->board, self->toPlay)){
        ChessBoard_unmakeMove(self->board);
        return 0;
    }
    if(self->afterGen!=NULL)
        (*self->afterGen)(self->board);
    self->next[(self->nextCount)++] = 
        ChessBoard_unmakeMove(self->board);
    return 1;
}
int __generatePawnPromotion(ChessMoveGenerator* self, int promoFlag,
    move_t baseMove, int validate){
    ChessBoard_makeMove(self->board, baseMove|promoFlag);
    return __finalizeAndUndo(self, validate);
}
void __generatePawnPromotions(ChessMoveGenerator* self, 
    move_t baseMove){
    baseMove|=PROMOTION_MOVE_FLAG;
    if(__generatePawnPromotion(self, QUEEN_PROMOTION, baseMove, 1)){
        __generatePawnPromotion(self, ROOK_PROMOTION, baseMove, 0);
        __generatePawnPromotion(self, KNIGHT_PROMOTION, baseMove, 0);
        __generatePawnPromotion(self, BISHOP_PROMOTION, baseMove, 0);
    }
}
void __generatePawnCapture(ChessMoveGenerator* self, 
    int rank, int file, int rankDelta, int fileDelta,
    int lastRank){
    location_t to = RANK_FILE(rank+rankDelta,file+fileDelta);
    ChessPiece* capture = self->board->squares[to];
    if(capture!=NULL && capture->color!=self->toPlay){
        location_t from = RANK_FILE(rank,file);
        move_t move = NEW_MOVE(from,to)|CAPTURE_MOVE_FLAG;
        if(rank==lastRank){
            //generate capture promotions
            __generatePawnPromotions(self, move);
        }
        else{
            ChessBoard_makeMove(self->board, move);
            __finalizeAndUndo(self, 1);
        }
    }
}
void __generatePawnMoves(ChessMoveGenerator* self){
    int i, rank, file;
    assert(self!=NULL);
    assert(self->curSet!=NULL);
    int size = self->curSet->piecesCounts[PAWN_INDEX];
    int pawnDirection = self->toPlay==WHITE?1:-1;
    int pawnHomeRank = self->toPlay==WHITE?1:6;
    int lastRank = self->toPlay==WHITE?6:1;
    ChessPiece* pawn;
    move_t move;
    location_t to;
    //since the order of the pawns might change in the set
    //durring operations, we must store a local copy
    ChessPiece** pawns = (ChessPiece**)malloc(
        sizeof(ChessPiece*)*size);
    memcpy(pawns, self->curSet->piecesByType[PAWN_INDEX], 
        sizeof(ChessPiece*)*size);
    for(i=0;i<size;i++){
        pawn = pawns[i];
        rank = GET_RANK(pawn->location);
        file = GET_FILE(pawn->location);
        //try to move forward
        to = RANK_FILE(rank+pawnDirection, file);
        if(self->board->squares[to]==NULL){
            move = NEW_MOVE(pawn->location,to);
            if(rank==lastRank){
                //generate promotions
                __generatePawnPromotions(self, move);
            }
            else{
                //generate regular forward push
                ChessBoard_makeMove(self->board, move);
                if((__finalizeAndUndo(self, 1)||self->inCheck)
                        && pawnHomeRank==rank){
                    to = RANK_FILE(rank+pawnDirection*2, file);
                    if(self->board->squares[to]==NULL){
                        move = NEW_MOVE(pawn->location,to);
                        move|=DOUBLE_PAWN_PUSH_MOVE;
                        ChessBoard_makeMove(self->board, move);
                        __finalizeAndUndo(self, self->inCheck);
                    }
                }
            }
        }
        if(file>0){//try to capture left
            __generatePawnCapture(self, rank, file, pawnDirection, -1,
                lastRank);
        }
        if(file<7){//try to capture right
            __generatePawnCapture(self, rank, file, pawnDirection, 1,
                lastRank);
        }
    }//end for 
    free(pawns);
}
void __generateDirectionalMoves(ChessMoveGenerator* self, 
    ChessPiece* piece, int dRank, int dFile){
    ChessPiece* capture;
    int rank, file;
    rank = GET_RANK(piece->location)+dRank;
    file = GET_FILE(piece->location)+dFile;
    int validated = 0;
    location_t to;
    move_t move;
    //while inside the board (0<=rank<8 && 0<=file<8)
    while(((rank&7)==rank) && ((file&7)==file)){
        to = RANK_FILE(rank,file);
        capture = self->board->squares[to];
        if(capture==NULL){
            move = NEW_MOVE(piece->location, to);
            ChessBoard_makeMove(self->board, move);
            if(__finalizeAndUndo(self, (!validated)||self->inCheck)){
                if(self->inCheck)
                    break; //move blocks some directional check, 
                           //any further move in this direction 
                           //will not.
                validated = 1;
            }
            else if(!self->inCheck){
                break; //move puts you in check, any move in
                       //this direction will also put you in check
            }
        }
        else if(capture->color != self->toPlay){
            move = NEW_MOVE(piece->location, to)|CAPTURE_MOVE_FLAG;
            ChessBoard_makeMove(self->board, move);
            __finalizeAndUndo(self, self->inCheck||(!validated));
            break;
        }
        else break; //you hit your own piece
        rank+=dRank;
        file+=dFile;
    }
}
void __generateBishopMoves(ChessMoveGenerator* self){
    int i;
    int size = self->curSet->piecesCounts[BISHOP_INDEX];
    ChessPiece* bishop;
    for(i=0;i<size;i++){
        bishop = self->curSet->piecesByType[BISHOP_INDEX][i];
        __generateDirectionalMoves(self, bishop, 1, 1);
        __generateDirectionalMoves(self, bishop, -1, 1);
        __generateDirectionalMoves(self, bishop, 1, -1);
        __generateDirectionalMoves(self, bishop, -1, -1);
    }
}
void __generateRookMoves(ChessMoveGenerator* self){
    int size = self->curSet->piecesCounts[ROOK_INDEX];
    int i;
    ChessPiece* rook;
    for(i=0;i<size;i++){
        rook = self->curSet->piecesByType[ROOK_INDEX][i];
        __generateDirectionalMoves(self, rook, 1, 0);
        __generateDirectionalMoves(self, rook, 0, 1);
        __generateDirectionalMoves(self, rook, -1, 0);
        __generateDirectionalMoves(self, rook, 0, -1);
    }
}
void __generateQueenMoves(ChessMoveGenerator* self){
    int i;
    int size = self->curSet->piecesCounts[QUEEN_INDEX];
    ChessPiece* queen;
    for(i=0;i<size;i++){
        queen = self->curSet->piecesByType[QUEEN_INDEX][i];
        __generateDirectionalMoves(self, queen, 1, 0);
        __generateDirectionalMoves(self, queen, 0, 1);
        __generateDirectionalMoves(self, queen, -1, 0);
        __generateDirectionalMoves(self, queen, 0, -1);
        __generateDirectionalMoves(self, queen, 1, 1);
        __generateDirectionalMoves(self, queen, -1, 1);
        __generateDirectionalMoves(self, queen, 1, -1);
        __generateDirectionalMoves(self, queen, -1, -1);
    }
}
int __generateSimpleMove(ChessMoveGenerator* self, ChessPiece* piece, 
    int dRank, int dFile, int validate){
    //returns 1 if the move was succesfull, 
    //0 if it failed because of check, 
    //and -1 if it failed because it was out of bounds/blocked
    ChessPiece* capture;
    int rank, file;
    location_t to;
    move_t move;
    rank = GET_RANK(piece->location)+dRank;
    file = GET_FILE(piece->location)+dFile;
    //if the move is in the board (0<=rank<8)&&(0<=file<8)
    if(((rank&7)==rank) && ((file&7)==file)){
        to = RANK_FILE(rank,file);
        move = NEW_MOVE(piece->location, to);
        capture = self->board->squares[to];
        if((capture!=NULL) && ((capture->color) != (self->toPlay)))
            move|=CAPTURE_MOVE_FLAG;
        else if(capture!=NULL)
            return -1; //the same color
        ChessBoard_makeMove(self->board, move);
        if(__finalizeAndUndo(self, validate)) return 1;
        else return 0;
    }
    return -1;
}
void __generateKnightMoves(ChessMoveGenerator* self){
    int i;
    int size = self->curSet->piecesCounts[KNIGHT_INDEX];
    ChessPiece* knight;
    for(i=0;i<size;i++){
        knight = self->curSet->piecesByType[KNIGHT_INDEX][i];
        int validated = __generateSimpleMove(self, knight, 2, -1, 1);
        if(validated==0 && !self->inCheck) continue;
        validated = __generateSimpleMove(self, knight, 2, 1, validated!=1 || self->inCheck);
        if(validated==0 && !self->inCheck) continue;
        validated = __generateSimpleMove(self, knight, 1, 2, validated!=1 || self->inCheck);
        if(validated==0 && !self->inCheck) continue;
        validated = __generateSimpleMove(self, knight, -1, 2, validated!=1 || self->inCheck);
        if(validated==0 && !self->inCheck) continue;
        validated = __generateSimpleMove(self, knight, -2, 1, validated!=1 || self->inCheck);
        if(validated==0 && !self->inCheck) continue;
        validated = __generateSimpleMove(self, knight, -2, -1, validated!=1 || self->inCheck);
        if(validated==0 && !self->inCheck) continue;
        validated = __generateSimpleMove(self, knight, -1, -2, validated!=1 || self->inCheck);
        if(validated==0 && !self->inCheck) continue;
        validated = __generateSimpleMove(self, knight, 1, -2, validated!=1 || self->inCheck);
    }
}
void __generateKingMoves(ChessMoveGenerator* self){
    ChessPiece* king = self->curSet->piecesByType[KING_INDEX][0];
    __generateSimpleMove(self, king, -1, -1, 1);
    __generateSimpleMove(self, king, -1, 0, 1);
    __generateSimpleMove(self, king, -1, 1, 1);
    __generateSimpleMove(self, king, 0, -1, 1);
    __generateSimpleMove(self, king, 0, 1, 1);
    __generateSimpleMove(self, king, 1, -1, 1);
    __generateSimpleMove(self, king, 1, 0, 1);
    __generateSimpleMove(self, king, 1, 1, 1);
}
void __generateEnPassant(ChessMoveGenerator* self){
    if(!(self->board->flags&EN_PASSANT_FLAG))
        return;
    int file = (self->board->flags>>EN_PASSANT_FILE_OFFSET)&7;
    int pawnDirection = self->toPlay==WHITE?1:-1;
    int enPassantFromRank = self->toPlay==WHITE?4:3;
    ChessPiece* pawn;
    location_t from, to;
    move_t move;
    if(file<7){
        from = RANK_FILE(enPassantFromRank, file+1);
        pawn = self->board->squares[from];
        if(pawn!=NULL && pawn->type==PAWN &&
                pawn->color==self->toPlay){
            to = RANK_FILE(enPassantFromRank+pawnDirection, file);
            move = NEW_MOVE(from, to)|EN_PASSANT_MOVE;
            ChessBoard_makeMove(self->board, move);
            __finalizeAndUndo(self, 1);
        }
    }
    if(file>0){
        from = RANK_FILE(enPassantFromRank, file-1);
        pawn = self->board->squares[from];
        if(pawn!=NULL && pawn->type==PAWN &&
                pawn->color==self->toPlay){
            to = RANK_FILE(enPassantFromRank+pawnDirection, file);
            move = NEW_MOVE(from, to)|EN_PASSANT_MOVE;
            ChessBoard_makeMove(self->board, move);
            __finalizeAndUndo(self, 1);
        }
    }
}
void __generateCastlings(ChessMoveGenerator* self){
    int homeRank = self->toPlay==WHITE?0:7;
    int canKingCastle = 0, canQueenCastle = 0;
    move_t move;
    location_t from, between, to;
    from = RANK_FILE(homeRank, 4);
    flag_t flags = self->board->flags;
    if(self->toPlay==WHITE){
        if(flags&WHITE_KING_CASTLE_FLAG)
            canKingCastle = 1;
        if(flags&WHITE_QUEEN_CASTLE_FLAG)
            canQueenCastle = 1;
    }
    else{
        if(flags&BLACK_KING_CASTLE_FLAG)
            canKingCastle = 1;
        if(flags&BLACK_QUEEN_CASTLE_FLAG)
            canQueenCastle = 1;
    }
    if(canKingCastle){
        between = RANK_FILE(homeRank, 5);
        if(self->board->squares[between]==NULL){
            to = RANK_FILE(homeRank, 6);
            if(self->board->squares[to]==NULL){
                move = NEW_MOVE(from,between);
                assert(self->board->squares[from]!=NULL);
                ChessBoard_makeMove(self->board, move);
                if(__testForCheck(self->board, self->toPlay))
                    ChessBoard_unmakeMove(self->board);
                else{
                    move = NEW_MOVE(from,to)|KING_CASTLE_MOVE;
                    ChessBoard_unmakeMove(self->board);
                    ChessBoard_makeMove(self->board, move);
                    __finalizeAndUndo(self, 1);
                }
            }
        }
    }
    if(canQueenCastle){
        between = RANK_FILE(homeRank, 3);
        if(self->board->squares[between]==NULL){
            to = RANK_FILE(homeRank, 2);
            if((self->board->squares[to]==NULL)&&(self->board
                    ->squares[RANK_FILE(homeRank, 1)]==NULL)){
                move = NEW_MOVE(from,between);
                ChessBoard_makeMove(self->board, move);
                if(__testForCheck(self->board, self->toPlay))
                    ChessBoard_unmakeMove(self->board);
                else{
                    move = NEW_MOVE(from,to)|QUEEN_CASTLE_MOVE;
                    ChessBoard_unmakeMove(self->board);
                    ChessBoard_makeMove(self->board, move);
                    __finalizeAndUndo(self, 1);
                }
            }
        }
    }
}
