#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "board.h"

ChessPiece* ChessPiece_new(color_e color, pieceType_e type){
    ChessPiece* ret = (ChessPiece*)malloc(sizeof(ChessPiece));
    ret->location = UNKNOWN_LOCATION;
    ret->color = color;
    ret->type = type;
    return ret;
}
void ChessPiece_delete(ChessPiece* self){
    free(self);
}
char ChessPiece_getChar(ChessPiece* self){
    switch(self->type){
    case KING:   return self->color==WHITE?'K':'k';
    case QUEEN:  return self->color==WHITE?'Q':'q';
    case ROOK:   return self->color==WHITE?'R':'r';
    case KNIGHT: return self->color==WHITE?'N':'n';
    case BISHOP: return self->color==WHITE?'B':'b';
    case PAWN:   return self->color==WHITE?'P':'p';
    default:
        assert(0);
        return '?';
    }
}
int ChessPiece_getZobristID(ChessPiece* self){
    int a;
    switch(self->type){
    case KING:   a = self->color==WHITE?  0: 64; break;
    case QUEEN:  a = self->color==WHITE?128:192; break;
    case ROOK:   a = self->color==WHITE?256:320; break;
    case KNIGHT: a = self->color==WHITE?384:448; break;
    case BISHOP: a = self->color==WHITE?512:576; break;
    case PAWN:   a = self->color==WHITE?640:704; break;
    default:
        assert(0);
        a = -1;
    }
    a+=(int)self->location;
    return a;
}

void ChessPieceSet_initWhite(ChessBoard* board){
    assert(board->whitePieces==NULL);
    ChessPieceSet* set = (ChessPieceSet*)malloc(sizeof(ChessPieceSet));
    board->whitePieces = set;

    set->king = ChessPiece_new(WHITE, KING);
    set->queensCount = 1;
    set->queens = (ChessPiece**)malloc(sizeof(ChessPiece*));
    set->queens[0] = ChessPiece_new(WHITE, QUEEN);
    set->rooksCount = 2;
    set->rooks = (ChessPiece**)malloc(sizeof(ChessPiece*)*2);
    set->rooks[0] = ChessPiece_new(WHITE, ROOK);
    set->rooks[1] = ChessPiece_new(WHITE, ROOK);
    set->knightsCount = 2;
    set->knights = (ChessPiece**)malloc(sizeof(ChessPiece*)*2);
    set->knights[0] = ChessPiece_new(WHITE, KNIGHT);
    set->knights[1] = ChessPiece_new(WHITE, KNIGHT);
    set->bishopsCount = 2;
    set->bishops = (ChessPiece**)malloc(sizeof(ChessPiece*)*2);
    set->bishops[0] = ChessPiece_new(WHITE, BISHOP);
    set->bishops[1] = ChessPiece_new(WHITE, BISHOP);
    set->pawnsCount = 8;
    set->pawns = (ChessPiece**)malloc(sizeof(ChessPiece*)*8);
    int i;
    for(i = 0; i < 8; i++)
        set->pawns[i] = ChessPiece_new(WHITE, PAWN);

    ChessBoard_setPiece(board, set->rooks[0],   RANK_FILE(0,0));
    ChessBoard_setPiece(board, set->knights[0], RANK_FILE(0,1));
    ChessBoard_setPiece(board, set->bishops[0], RANK_FILE(0,2));
    ChessBoard_setPiece(board, set->queens[0],  RANK_FILE(0,3));
    ChessBoard_setPiece(board, set->king,       RANK_FILE(0,4));
    ChessBoard_setPiece(board, set->bishops[1], RANK_FILE(0,5));
    ChessBoard_setPiece(board, set->knights[1], RANK_FILE(0,6));
    ChessBoard_setPiece(board, set->rooks[1],   RANK_FILE(0,7));
    for(i=0; i < 8; i++)
        ChessBoard_setPiece(board, set->pawns[i],   RANK_FILE(1,i));
}
void ChessPieceSet_initBlack(ChessBoard* board){
    assert(board->blackPieces==NULL);
    ChessPieceSet* set = (ChessPieceSet*)malloc(sizeof(ChessPieceSet));
    board->blackPieces = set;

    set->king = ChessPiece_new(BLACK, KING);
    set->queensCount = 1;
    set->queens = (ChessPiece**)malloc(sizeof(ChessPiece*));
    set->queens[0] = ChessPiece_new(BLACK, QUEEN);
    set->rooksCount = 2;
    set->rooks = (ChessPiece**)malloc(sizeof(ChessPiece*)*2);
    set->rooks[0] = ChessPiece_new(BLACK, ROOK);
    set->rooks[1] = ChessPiece_new(BLACK, ROOK);
    set->knightsCount = 2;
    set->knights = (ChessPiece**)malloc(sizeof(ChessPiece*)*2);
    set->knights[0] = ChessPiece_new(BLACK, KNIGHT);
    set->knights[1] = ChessPiece_new(BLACK, KNIGHT);
    set->bishopsCount = 2;
    set->bishops = (ChessPiece**)malloc(sizeof(ChessPiece*)*2);
    set->bishops[0] = ChessPiece_new(BLACK, BISHOP);
    set->bishops[1] = ChessPiece_new(BLACK, BISHOP);
    set->pawnsCount = 8;
    set->pawns = (ChessPiece**)malloc(sizeof(ChessPiece*)*8);
    int i;
    for(i = 0; i < 8; i++)
        set->pawns[i] = ChessPiece_new(BLACK, PAWN);

    ChessBoard_setPiece(board, set->rooks[0],   RANK_FILE(7,0));
    ChessBoard_setPiece(board, set->knights[0], RANK_FILE(7,1));
    ChessBoard_setPiece(board, set->bishops[0], RANK_FILE(7,2));
    ChessBoard_setPiece(board, set->queens[0],  RANK_FILE(7,3));
    ChessBoard_setPiece(board, set->king,       RANK_FILE(7,4));
    ChessBoard_setPiece(board, set->bishops[1], RANK_FILE(7,5));
    ChessBoard_setPiece(board, set->knights[1], RANK_FILE(7,6));
    ChessBoard_setPiece(board, set->rooks[1],   RANK_FILE(7,7));
    for(i=0; i < 8; i++)
        ChessBoard_setPiece(board, set->pawns[i],   RANK_FILE(6,i));
}
void ChessPieceSet_delete(ChessPieceSet* self){
    ChessPiece_delete(self->king);
    int i;
    for(i=0; i<self->queensCount; i++)
        ChessPiece_delete(self->queens[i]);
    for(i=0; i<self->rooksCount; i++)
        ChessPiece_delete(self->rooks[i]);
    for(i=0; i<self->knightsCount; i++)
        ChessPiece_delete(self->knights[i]);
    for(i=0; i<self->bishopsCount; i++)
        ChessPiece_delete(self->bishops[i]);
    for(i=0; i<self->pawnsCount; i++)
        ChessPiece_delete(self->pawns[i]);
    free(self->queens);
    free(self->rooks);
    free(self->knights);
    free(self->bishops);
    free(self->pawns);
    free(self);
}

ChessBoard* ChessBoard_new(){
    ChessBoard* board = (ChessBoard*)malloc(sizeof(ChessBoard));
    int i;
    for(i=0; i<64; i++)
        board->squares[i]=NULL;
    board->whitePieces=NULL;
    board->blackPieces=NULL;
    ChessPieceSet_initWhite(board);
    ChessPieceSet_initBlack(board);
    return board;
}
void ChessBoard_delete(ChessBoard* self){
    ChessPieceSet_delete(self->whitePieces);
    ChessPieceSet_delete(self->blackPieces);
    free(self);
}
char _ChessBoard_getChar(ChessBoard* self, location_t loc){
    assert(self!=NULL);
    assert(0<=loc);
    assert(loc<64);
    ChessPiece* piece = self->squares[loc];
    if(piece==NULL)
        return ((GET_RANK(loc)^GET_FILE(loc))%2)?'+':'#';
    return ChessPiece_getChar(piece);
}
void ChessBoard_setPiece(ChessBoard* self, ChessPiece* piece,
    location_t loc){
    assert(piece->location==UNKNOWN_LOCATION);
    assert(0<=loc);
    assert(loc<64);
    assert(self->squares[loc]==NULL);
    self->squares[loc] = piece;
    piece->location = loc;
}
void ChessBoard_print(ChessBoard* self){
    int r,f;
    for(r=7; r>=-1; r--){
        for(f=-1; f<8; f++){
            if(r==-1 && f==-1)
                printf("  ");
            else if(r==-1)
                printf("%c ", 'a'+f);
            else if(f==-1)
                printf("%d ", r+1);
            else{
                printf("%c ", 
                    _ChessBoard_getChar(self, RANK_FILE(r,f))
                );
            }
        }
        printf("\n");
    }
}
