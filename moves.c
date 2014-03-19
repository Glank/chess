#include "moves.h"

void __initIterValues(ChessMoveGenerator* self);
void __generatePawnMoves(ChessMoveGenerator* self);
int __generatePawnPromotion(ChessMoveGenerator* self, ChessBoard* setup, pieceType_e type,
    location_t location, int validate);
void __generatePawnPromotions(ChessMoveGenerator* self, ChessBoard* setup, ChessPiece* freePiece);
void __generatePawnCapture(ChessMoveGenerator* self, ChessPiece* pawn, int rank, int file, int dir);
void __generateDirectionalMoves(ChessMoveGenerator* self, Piece* piece, int dRank, int dFile);
void __generateBishopMoves(ChessMoveGenerator* self);
void __generateKnightMoves(ChessMoveGenerator* self);
void __generateRookMoves(ChessMoveGenerator* self);
void __generateQueenMoves(ChessMoveGenerator* self);
void __generateKingMoves(ChessMoveGenerator* self);
void __generateCastlings(ChessMoveGenerator* self);
void __generateEnPassant(ChessMoveGenerator* self);
int __finalizeOrDelete(ChessMoveGenerator* self, ChessBoard* board);
int __testForCheck(ChessBoard* board, color_e color);
ChessBoard* __getCleanBoard(ChessMoveGenerator* self);

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
    }
    else{
        self->inCheck = flags&BLACK_IN_CHECK;
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
    int i, rankDelta, fileDelta, rank, file;
    int size = opPieces->piecesCounts[TYPE_TO_INT(KNIGHT)];
    ChessPiece* knight;
    for(i=0; i<size; i++){
        knight = opPices->piecesByType[TYPE_TO_INT(KNIGHT)][i];
        rankDelta = kingRank-GET_RANK(knight->location);
        fileDelta = kingFile-GET_FILE(knight->location);
        if((rankDelta*rankDelta+fileDelta*fileDelta)==5)
            return 1;
    }
    //test for check by bishop
    size = opPieces->piecesCounts[TYPE_TO_INT(BISHOP)];
    ChessPiece* bishop;
    for(i=0; i<size; i++){
        bishop = opPices->piecesByType[TYPE_TO_INT(BISHOP)][i];
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
            if(rank==kingRank)
                return 1;
        }
    }
    //test for check by rook
    size = opPieces->piecesCounts[TYPE_TO_INT(ROOK)];
    ChessPiece* rook;
    for(i=0; i<size; i++){
        rook = opPices->piecesByType[TYPE_TO_INT(ROOK)][i];
        rankDelta = kingRank-GET_RANK(rook->location);
        fileDelta = kingFile-GET_FILE(rook->location);
        if((rankDelta == 0)||(fileDelta == 0)){
            //the king is on a rank or a file with this opposing rook 
            //check every square inbetween to see if the rook is blocked 
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
            if(rank==kingRank && file==kingFile)
                return 1;
        }
    }
    //test for check by queen
    size = opPieces->piecesCounts[TYPE_TO_INT(QUEEN)];
    ChessPiece* queen;
    for(i=0; i<size; i++){
        queen = opPices->piecesByType[TYPE_TO_INT(QUEEN)][i];
        rankDelta = kingRank-GET_RANK(queen->location);
        fileDelta = kingFile-GET_FILE(queen->location);
        if((rankDelta*rankDelta == fileDelta*fileDelta)||
            (rankDelta == 0)||(fileDelta == 0)){
            //the king is on a diagonal, rank or file with this opposing queen
            //check every square inbetween to see if the rook is blocked 
            if(fileDelta!=0) fileDelta = fileDelta>0?1:-1;
            if(rankDelta!=0) rankDelta = rankDelta>0?1:-1;
            rank = GET_RANK(queen->location)+rankDelta;
            file = GET_FILE(queen->location)+fileDelta;
            while(rank!=kingRank && file!=kingFile){
                if(board->squares[RANK_FILE(rank,file)]!=NULL)
                    break;
                rank+=rankDelta;
                file+=fileDelta;
            }
            if(rank==kingRank && file==kingFile)
                return 1;
        }
    }
    //test for check by other king
    //for hypothetical moves that are blocked
    //by the opposing king
    ChessPiece* opKing = opPices->piecesByType[TYPE_TO_INT(KING)][0];
    rankDelta = kingRank-GET_RANK(opKing->location);
    fileDelta = kingFile-GET_FILE(opKing->location);
    if((rankDelta*rankDelta+fileDelta*fileDelta)<=2)
        return 1;
    //Otherwise, the king is not in check
    return 0;
}

int __finalizeOrDelete(ChessMoveGenerator* self, ChessBoard* board, int validate){
    if(!validate || __testForCheck(board, self->toPlay)){
        ChessBoard_delete(board);
        return 0;
    }
    if(__testForcheck(board, OTHER_COLOR(self->toPlay))){
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
int __generatePawnPromotion(ChessMoveGenerator* self, ChessBoard* setup, pieceType_e type,
    location_t location, int validate){
    ChessBoard* clone = ChessBoard_clone(setup);
    ChessPiece* promoted = ChessPiece_new(self->toPlay, type);
    promoted->location = location;
    ChessPieceSet* set = self->toPlay==WHITE?clone->whitePieces:clone->blackPieces;
    ChessPieceSet_add(set, promoted, clone);
    return __finalizeOrDelete(self, clone, validate);
}
void __generatePawnPromotions(ChessMoveGenerator* self, ChessBoard* setup, ChessPiece* freePiece){
    if(__generatePawnPromotion(self, setup, QUEEN, freePiece->location, 1)){
        __generatePawnPromotion(self, setup, ROOK, freePiece->location, 0);
        __generatePawnPromotion(self, setup, KNIGHT, freePiece->location, 0);
        //gen bishop
        freePiece->type = BISHOP;
        ChessPieceSet* set = self->toPlay==WHITE?clone->whitePieces:clone->blackPieces;
        ChessPieceSet_add(set, freePiece, setup);
        __finalizeOrDelete(self, setup, 0);
    }
}
void __generatePawnCapture(ChessMoveGenerator* self, ChessPiece* pawn, int rank, int file, int dir){
    ChessPiece* capture, promoted;
    ChessPieceSet* set, opSet;
    capture = self->currentBoard->squares[
        RANK_FILE(rank+pawnDirection, file+dir)];
    if(capture!=NULL and capture->color!=self->toPlay){
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
            promoted->location = RANK_FILE(rank+pawnDirection, file);
            __generatePawnPromotions(self, clone, promoted)
        }
        else{
            //do normal capture
            clone = __getCleanBoard(self);
            opSet = self->toPlay==WHITE?clone->blackPieces:clone->whitePieces;
            capture = clone->squares[
                RANK_FILE(rank+pawnDirection, file+dir)];
            ChessPieceSet_remove(opSet, capture, clone);
            ChessPiece_delete(capture);
            ChessBoard_movePieceByLoc(clone, pawn->location, 
                RANK_FILE(rank+pawnDirection, file));
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
    ChessPiece* pawn, promoted;
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
                set = self->toPlay==WHITE?clone->whitePieces:clone->blackPieces;
                ChessPieceSet_remove(set, promoted, clone);
                promoted->location = RANK_FILE(rank+pawnDirection, file);
                __generatePawnPromotions(self, clone, promoted);
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
            __generatePawnCapture(self, pawn, rank, file, -1);
        //try to capture right
        if(file<7)
            __generatePawnCapture(self, pawn, rank, file, -1);
    }//end for 
}
void __generateDirectionalMoves(ChessMoveGenerator* self, Piece* piece, int dRank, int dFile){
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
        }
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
    int i;
    int size = self->curSet->piecesCounts[TYPE_TO_INT(ROOK)];
    ChessPiece* rook;
    for(i=0;i<size;i++){
        rook = self->curSet->piecesByType[TYPE_TO_INT(ROOK)][i];
        __generateDirectionalMoves(self, rook, 1, 0);
        __generateDirectionalMoves(self, rook, 0, 1);
        __generateDirectionalMoves(self, rook, -1, 0);
        __generateDirectionalMoves(self, rook, 0, -1);
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
