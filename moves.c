#include <stdio.h>
#include <stdlib.h>
#include "moves.h"

void __initIterValues(ChessMoveGenerator* self);
void __generatePawnMoves(ChessMoveGenerator* self);
int __generatePawnPromotion(ChessMoveGenerator* self, 
    ChessBoard* setup, pieceType_e type,
    location_t location, int validate);
void __generatePawnPromotions(ChessMoveGenerator* self, 
    ChessBoard* setup, ChessPiece* freePiece,
    location_t location);
void __generatePawnCapture(ChessMoveGenerator* self, ChessPiece* pawn,
    int rank, int file, int dir, 
    int pawnDirection, int lastRank);
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
void __finish(ChessMoveGenerator* self);

ChessMoveGenerator* ChessMoveGenerator_new(){
    ChessMoveGenerator* self = (ChessMoveGenerator*)
        malloc(sizeof(ChessMoveGenerator));
    self->tempNext = (ChessBoard**)malloc(
        sizeof(ChessBoard*)*MOVE_GEN_MAX_ALLOCATED);
    self->tempNextFilled = 0;
    self->cloneTemplate = NULL;
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
    __generateEnPassant(self);
    __generateCastlings(self);
    __finish(self);
    
}

void __finish(ChessMoveGenerator* self){
    int i;
    self->currentBoard->nextCount = self->tempNextFilled;
    self->currentBoard->next = (ChessBoard**)malloc(
        sizeof(ChessBoard*)*self->tempNextFilled);
    ChessBoard* next;
    for(i = 0; i < self->tempNextFilled; i++){
        next = self->tempNext[i];
        next->previous = self->currentBoard;
        self->currentBoard->next[i] = next;
    }
    if(self->cloneTemplate!=NULL){
        ChessBoard_delete(self->cloneTemplate);
        self->cloneTemplate = NULL;
    }
}

void __initIterValues(ChessMoveGenerator* self){
    self->tempNextFilled = 0;
    flag_t flags = self->currentBoard->flags;
    self->toPlay = flags&TO_PLAY_FLAG?BLACK:WHITE;
    if(self->toPlay==WHITE){
        self->inCheck = flags&WHITE_IN_CHECK_FLAG;
        self->curSet = self->currentBoard->whitePieces;
    }
    else{
        self->inCheck = flags&BLACK_IN_CHECK_FLAG;
        self->curSet = self->currentBoard->blackPieces;
    }
    ChessBoard* clone = ChessBoard_clone(self->currentBoard);
    ChessBoard_clearEnPassantFlags(clone);
    ChessBoard_toggleToPlay(clone);
    self->cloneTemplate = clone;
}

int __testForCheck(ChessBoard* board, color_e color){
    ChessPieceSet* pieces = color==WHITE?
        board->whitePieces:board->blackPieces;
    ChessPieceSet* opPieces = color==WHITE?
        board->blackPieces:board->whitePieces;
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
    int size = opPieces->piecesCounts[TYPE_TO_INT(KNIGHT)];
    ChessPiece* knight;
    for(i=0; i<size; i++){
        knight = opPieces->piecesByType[TYPE_TO_INT(KNIGHT)][i];
        rankDelta = kingRank-GET_RANK(knight->location);
        fileDelta = kingFile-GET_FILE(knight->location);
        if((rankDelta*rankDelta+fileDelta*fileDelta)==5)
            return 1;
    }
    //test for check by bishop
    size = opPieces->piecesCounts[TYPE_TO_INT(BISHOP)];
    ChessPiece* bishop;
    for(i=0; i<size; i++){
        bishop = opPieces->piecesByType[TYPE_TO_INT(BISHOP)][i];
        rankDelta = kingRank-GET_RANK(bishop->location);
        fileDelta = kingFile-GET_FILE(bishop->location);
        if(rankDelta*rankDelta == fileDelta*fileDelta){
            //the king is on a diagonal with this opposing bishop
            //check every square inbetween to see if the bishop is blocked 
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
            if(rank==kingRank){
                return 1;
            }
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
            while(rank!=kingRank && file!=kingFile){
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

int __finalizeOrDelete(ChessMoveGenerator* self, 
    ChessBoard* board, int validate){
    if(validate && __testForCheck(board, self->toPlay)){
        ChessBoard_delete(board);
        return 0;
    }
    if(__testForCheck(board, OTHER_COLOR(self->toPlay))){
        if(self->toPlay==WHITE)
            board->flags = board->flags|BLACK_IN_CHECK_FLAG;
        else
            board->flags = board->flags|WHITE_IN_CHECK_FLAG;
    }
    self->tempNext[self->tempNextFilled++] = board;
    return 1;
}
ChessBoard* __getCleanBoard(ChessMoveGenerator* self){
    return ChessBoard_clone(self->cloneTemplate);
}
int __generatePawnPromotion(ChessMoveGenerator* self, 
    ChessBoard* setup, pieceType_e type,
    location_t location, int validate){
    ChessBoard* clone = ChessBoard_clone(setup);
    ChessPiece* promoted = ChessPiece_new(self->toPlay, type);
    ChessPieceSet* set = self->toPlay==WHITE?
        clone->whitePieces:clone->blackPieces;
    ChessPieceSet_add(set, promoted, clone, location);
    return __finalizeOrDelete(self, clone, validate);
}
void __generatePawnPromotions(ChessMoveGenerator* self,
    ChessBoard* setup, ChessPiece* freePiece,
    location_t location){
    if(__generatePawnPromotion(self, setup, QUEEN, location, 1)){
        __generatePawnPromotion(self, setup, ROOK, location, 0);
        __generatePawnPromotion(self, setup, KNIGHT, location, 0);
        //gen bishop
        freePiece->type = BISHOP;
        ChessPieceSet* set = self->toPlay==WHITE?
            setup->whitePieces:setup->blackPieces;
        ChessPieceSet_add(set, freePiece, setup, location);
        __finalizeOrDelete(self, setup, 0);
    }
}
void __generatePawnCapture(ChessMoveGenerator* self, ChessPiece* pawn,
    int rank, int file, int dir, 
    int pawnDirection, int lastRank){
    ChessBoard* clone;
    ChessPiece* capture, *promoted;
    ChessPieceSet* set, *opSet;
    capture = self->currentBoard->squares[
        RANK_FILE(rank+pawnDirection, file+dir)];
    if(capture!=NULL && capture->color!=self->toPlay){
        if(rank==lastRank){
            //generate capture promotions
            clone = __getCleanBoard(self);
            promoted = clone->squares[pawn->location];
            capture = clone->squares[
                RANK_FILE(rank+pawnDirection, file+dir)];
            if(self->toPlay==WHITE){
                set = clone->whitePieces;
                opSet = clone->blackPieces;
            }
            else{
                set = clone->blackPieces;
                opSet = clone->whitePieces;
            }
            ChessPieceSet_remove(set, promoted, clone);
            ChessPieceSet_remove(opSet, capture, clone);
            ChessPiece_delete(capture);
            promoted->location = UNKNOWN_LOCATION;
            __generatePawnPromotions(self, clone, promoted, 
                RANK_FILE(rank+pawnDirection, file+dir));
        }
        else{
            //do normal capture
            clone = __getCleanBoard(self);
            opSet = self->toPlay==WHITE?
                clone->blackPieces:clone->whitePieces;
            capture = clone->squares[
                RANK_FILE(rank+pawnDirection, file+dir)];
            ChessPieceSet_remove(opSet, capture, clone);
            ChessPiece_delete(capture);
            ChessBoard_movePieceByLoc(clone, pawn->location, 
                RANK_FILE(rank+pawnDirection, file+dir));
            __finalizeOrDelete(self, clone, 1);
        }
    }
}
void __generatePawnMoves(ChessMoveGenerator* self){
    int i, rank, file;
    int size = self->curSet->piecesCounts[TYPE_TO_INT(PAWN)];
    int pawnDirection = self->toPlay==WHITE?1:-1;
    int pawnHomeRank = self->toPlay==WHITE?1:6;
    int lastRank = self->toPlay==WHITE?6:1;
    ChessPiece* pawn, *promoted;
    ChessPieceSet* set;
    ChessBoard* clone;
    for(i=0;i<size;i++){
        pawn = self->curSet->piecesByType[TYPE_TO_INT(PAWN)][i];
        rank = GET_RANK(pawn->location);
        file = GET_FILE(pawn->location);
        //try to move forward
        if(self->currentBoard->squares[
            RANK_FILE(rank+pawnDirection, file)]==NULL){
            if(rank==lastRank){
                //generate promotions
                clone = __getCleanBoard(self);
                promoted = clone->squares[pawn->location];
                set = self->toPlay==WHITE?
                    clone->whitePieces:clone->blackPieces;
                ChessPieceSet_remove(set, promoted, clone);
                promoted->location = UNKNOWN_LOCATION;
                __generatePawnPromotions(self, clone, promoted,
                    RANK_FILE(rank+pawnDirection, file));
            }
            else{
                //generate regular forward push
                clone = __getCleanBoard(self);
                ChessBoard_movePieceByLoc(clone, pawn->location, 
                    RANK_FILE(rank+pawnDirection, file));
                //and try to move forward 2 if in home rank
                if(__finalizeOrDelete(self, clone, 1) &&
                    pawnHomeRank==rank && self->currentBoard->squares[
                    RANK_FILE(rank+pawnDirection*2, file)]==NULL){
                    clone = __getCleanBoard(self);
                    ChessBoard_movePieceByLoc(clone, pawn->location, 
                        RANK_FILE(rank+pawnDirection*2, file));
                    ChessBoard_setEnPassantFlags(clone, file);
                    __finalizeOrDelete(self, clone, self->inCheck);
                }
            }
        }
        //try to capture left
        if(file>0)
            __generatePawnCapture(self, pawn, rank, file, -1, pawnDirection, lastRank);
        //try to capture right
        if(file<7)
            __generatePawnCapture(self, pawn, rank, file, 1, pawnDirection, lastRank);
    }//end for 
}
void __generateDirectionalMoves(ChessMoveGenerator* self, ChessPiece* piece, int dRank, int dFile){
    ChessBoard* clone;
    ChessPieceSet* opSet;
    ChessPiece* capture;
    int rank, file;
    rank = GET_RANK(piece->location)+dRank;
    file = GET_FILE(piece->location)+dFile;
    int validated = 0;
    while(((rank&7)==rank) && ((file&7)==file)){
        capture = self->currentBoard->squares[RANK_FILE(rank,file)];
        if(capture==NULL){
            clone = __getCleanBoard(self);
            ChessBoard_movePieceByLoc(clone, piece->location, 
                RANK_FILE(rank, file));
            if(__finalizeOrDelete(self, clone, self->inCheck||(!validated))){
                if(self->inCheck)
                    break;
                validated = 1;
            }
            else if(!self->inCheck)
                break;
        }
        else if(capture->color != self->toPlay){
            clone = __getCleanBoard(self);
            capture = clone->squares[RANK_FILE(rank,file)];
            opSet = self->toPlay==WHITE?clone->blackPieces:clone->whitePieces;
            ChessPieceSet_remove(opSet, capture, clone);
            ChessPiece_delete(capture);
            ChessBoard_movePieceByLoc(clone, piece->location, 
                RANK_FILE(rank, file));
            __finalizeOrDelete(self, clone, self->inCheck||(!validated));
            break;
        }
        else break; //you hit your own piece
        rank+=dRank;
        file+=dFile;
    }
}
void __generateBishopMoves(ChessMoveGenerator* self){
    int i;
    int size = self->curSet->piecesCounts[TYPE_TO_INT(BISHOP)];
    ChessPiece* bishop;
    for(i=0;i<size;i++){
        bishop = self->curSet->piecesByType[TYPE_TO_INT(BISHOP)][i];
        __generateDirectionalMoves(self, bishop, 1, 1);
        __generateDirectionalMoves(self, bishop, -1, 1);
        __generateDirectionalMoves(self, bishop, 1, -1);
        __generateDirectionalMoves(self, bishop, -1, -1);
    }
}
void __generateRookMoves(ChessMoveGenerator* self){
    int flagsBackup = self->cloneTemplate->flags;
    int size = self->curSet->piecesCounts[TYPE_TO_INT(ROOK)];
    int homeRank = self->toPlay==WHITE?0:7;
    int i, rank, file;
    ChessPiece* rook;
    for(i=0;i<size;i++){
        rook = self->curSet->piecesByType[TYPE_TO_INT(ROOK)][i];
        rank = GET_RANK(rook->location);
        file = GET_FILE(rook->location);
        if(rank==homeRank){
            if(file==0){
                if(self->toPlay==WHITE)
                    self->cloneTemplate->flags&=
                        ~WHITE_QUEEN_CASTLE_FLAG;
                else
                    self->cloneTemplate->flags&=
                        ~BLACK_QUEEN_CASTLE_FLAG;
            }
            else if(file==7){
                if(self->toPlay==WHITE)
                    self->cloneTemplate->flags&=
                        ~WHITE_KING_CASTLE_FLAG;
                else
                    self->cloneTemplate->flags&=
                        ~BLACK_KING_CASTLE_FLAG;
            }
        }
        __generateDirectionalMoves(self, rook, 1, 0);
        __generateDirectionalMoves(self, rook, 0, 1);
        __generateDirectionalMoves(self, rook, -1, 0);
        __generateDirectionalMoves(self, rook, 0, -1);
        self->cloneTemplate->flags = flagsBackup;
    }
}
void __generateQueenMoves(ChessMoveGenerator* self){
    int i;
    int size = self->curSet->piecesCounts[TYPE_TO_INT(QUEEN)];
    ChessPiece* queen;
    for(i=0;i<size;i++){
        queen = self->curSet->piecesByType[TYPE_TO_INT(QUEEN)][i];
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
    ChessBoard* clone;
    ChessPieceSet* opSet;
    ChessPiece* capture;
    int rank, file;
    rank = GET_RANK(piece->location)+dRank;
    file = GET_FILE(piece->location)+dFile;
    if(((rank&7)==rank) && ((file&7)==file)){
        capture = self->currentBoard->squares[RANK_FILE(rank,file)];
        if((capture!=NULL) && (capture->color != self->toPlay)){
            clone = __getCleanBoard(self);
            capture = clone->squares[RANK_FILE(rank,file)];
            opSet = self->toPlay==WHITE?
                clone->blackPieces:clone->whitePieces;
            ChessPieceSet_remove(opSet, capture, clone);
            ChessPiece_delete(capture);
        }
        else if(capture!=NULL)
            return -1;
        else
            clone = __getCleanBoard(self);
        ChessBoard_movePieceByLoc(clone, piece->location, 
            RANK_FILE(rank, file));
        if(__finalizeOrDelete(self, clone, validate)) return 1;
        else return 0;
    }
    return -1;
}
void __generateKnightMoves(ChessMoveGenerator* self){
    int i;
    int size = self->curSet->piecesCounts[TYPE_TO_INT(KNIGHT)];
    ChessPiece* knight;
    for(i=0;i<size;i++){
        knight = self->curSet->piecesByType[TYPE_TO_INT(KNIGHT)][i];
        if(__generateSimpleMove(self, knight, 2, -1, 1)==0 &&
            !self->inCheck) continue;
        __generateSimpleMove(self, knight, 2, 1, self->inCheck);
        __generateSimpleMove(self, knight, 1, 2, self->inCheck);
        __generateSimpleMove(self, knight, -1, 2, self->inCheck);
        __generateSimpleMove(self, knight, -2, 1, self->inCheck);
        __generateSimpleMove(self, knight, -2, -1, self->inCheck);
        __generateSimpleMove(self, knight, -1, -2, self->inCheck);
        __generateSimpleMove(self, knight, 1, -2, self->inCheck);
    }
}
void __generateKingMoves(ChessMoveGenerator* self){
    ChessPiece* king = self->curSet->
        piecesByType[TYPE_TO_INT(KING)][0];
    int flagsBackup = self->cloneTemplate->flags;
    if(self->toPlay==WHITE)
        self->cloneTemplate->flags&=~
            (WHITE_QUEEN_CASTLE_FLAG|WHITE_KING_CASTLE_FLAG);
    else
        self->cloneTemplate->flags&=~
            (BLACK_QUEEN_CASTLE_FLAG|BLACK_KING_CASTLE_FLAG);
    __generateSimpleMove(self, king, -1, -1, 1);
    __generateSimpleMove(self, king, -1, 0, 1);
    __generateSimpleMove(self, king, -1, 1, 1);
    __generateSimpleMove(self, king, 0, -1, 1);
    __generateSimpleMove(self, king, 0, 1, 1);
    __generateSimpleMove(self, king, 1, -1, 1);
    __generateSimpleMove(self, king, 1, 0, 1);
    __generateSimpleMove(self, king, 1, 1, 1);
    self->cloneTemplate->flags = flagsBackup;
}
void __generateEnPassant(ChessMoveGenerator* self){
    if(!(self->currentBoard->flags&EN_PASSANT_FLAG))
        return;
    int file = (self->currentBoard->flags>>EN_PASSANT_FILE_OFFSET)&7;
    int pawnDirection = self->toPlay==WHITE?1:-1;
    int enPassantFromRank = self->toPlay==WHITE?4:3;
    ChessBoard* clone;
    ChessPiece* pawn;
    if(file<7){
        pawn = self->currentBoard->squares[
            RANK_FILE(enPassantFromRank, file+1)];
        if(pawn!=NULL && pawn->type==PAWN &&
            pawn->color==self->toPlay){
            clone = __getCleanBoard(self);
            ChessBoard_quickRemoveByLoc(clone, 
                RANK_FILE(enPassantFromRank, file));
            ChessBoard_movePieceByLoc(clone,
                pawn->location,
                RANK_FILE(enPassantFromRank+pawnDirection, file));
            __finalizeOrDelete(self, clone, 1);
        }
    }
    if(file>0){
        pawn = self->currentBoard->squares[
            RANK_FILE(enPassantFromRank, file-1)];
        if(pawn!=NULL && pawn->type==PAWN &&
            pawn->color==self->toPlay){
            clone = __getCleanBoard(self);
            ChessBoard_quickRemoveByLoc(clone, 
                RANK_FILE(enPassantFromRank, file));
            ChessBoard_movePieceByLoc(clone,
                pawn->location,
                RANK_FILE(enPassantFromRank+pawnDirection, file));
            __finalizeOrDelete(self, clone, 1);
        }
    }
}
void __generateCastlings(ChessMoveGenerator* self){
    int flagsBackup = self->cloneTemplate->flags;
    if(self->toPlay==WHITE)
        self->cloneTemplate->flags&=~
            (WHITE_QUEEN_CASTLE_FLAG|WHITE_KING_CASTLE_FLAG);
    else
        self->cloneTemplate->flags&=~
            (BLACK_QUEEN_CASTLE_FLAG|BLACK_KING_CASTLE_FLAG);
    ChessBoard* clone;
    int homeRank = self->toPlay==WHITE?0:7;
    int canKingCastle = 0, canQueenCastle = 0;
    if(self->toPlay==WHITE){
        if(self->currentBoard->flags&WHITE_KING_CASTLE_FLAG)
            canKingCastle = 1;
        if(self->currentBoard->flags&WHITE_QUEEN_CASTLE_FLAG)
            canQueenCastle = 1;
    }
    else{
        if(self->currentBoard->flags&BLACK_KING_CASTLE_FLAG)
            canKingCastle = 1;
        if(self->currentBoard->flags&BLACK_QUEEN_CASTLE_FLAG)
            canQueenCastle = 1;
    }
    if(canKingCastle){
        if((self->currentBoard->
            squares[RANK_FILE(homeRank, 5)]==NULL)&&
            (self->currentBoard->
            squares[RANK_FILE(homeRank, 6)]==NULL)){
            clone = __getCleanBoard(self);
            ChessBoard_movePieceByLoc(clone,
                RANK_FILE(homeRank,4), RANK_FILE(homeRank,5));
            if(__testForCheck(clone, self->toPlay))
                ChessBoard_delete(clone);
            else{
                ChessBoard_movePieceByLoc(clone,
                    RANK_FILE(homeRank,5), RANK_FILE(homeRank,6));
                ChessBoard_movePieceByLoc(clone,
                    RANK_FILE(homeRank,7), RANK_FILE(homeRank,5));
                __finalizeOrDelete(self, clone, 1);
            }
        }
    }
    if(canQueenCastle){
        if((self->currentBoard->
            squares[RANK_FILE(homeRank, 3)]==NULL)&&
            (self->currentBoard->
            squares[RANK_FILE(homeRank, 2)]==NULL)&&
            (self->currentBoard->
            squares[RANK_FILE(homeRank, 1)]==NULL)){
            clone = __getCleanBoard(self);
            ChessBoard_movePieceByLoc(clone,
                RANK_FILE(homeRank,4), RANK_FILE(homeRank,3));
            if(__testForCheck(clone, self->toPlay))
                ChessBoard_delete(clone);
            else{
                ChessBoard_movePieceByLoc(clone,
                    RANK_FILE(homeRank,3), RANK_FILE(homeRank,2));
                ChessBoard_movePieceByLoc(clone,
                    RANK_FILE(homeRank,0), RANK_FILE(homeRank,3));
                __finalizeOrDelete(self, clone, 1);
            }
        }
    }
    self->cloneTemplate->flags = flagsBackup;
}
