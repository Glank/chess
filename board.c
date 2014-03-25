#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "board.h"

ChessPiece* ChessPiece_new(color_e color, pieceType_e type){
    ChessPiece* self = (ChessPiece*)malloc(sizeof(ChessPiece));
    self->location = UNKNOWN_LOCATION;
    self->color = color;
    self->type = type;
    return self;
}
void ChessPiece_delete(ChessPiece* self){
    free(self);
}
//get a single character representing this piece
char __getPieceChar(ChessPiece* self){
    char table[] = "KkQqRrNnBbPp";
    int i = TYPE_COLOR_TO_INT(self->type, self->color);
    return table[i];
}
//get a unique integer index in the ZOBRIST_TABLE
int __getZobristID(ChessPiece* self){
    int i = TYPE_COLOR_TO_INT(self->type, self->color);
    i=(i<<6)+(int)self->location;
    return i;
}

void __initSide(ChessBoard* board, color_e color){
    ChessPieceSet* set = ChessPieceSet_new();
    int backRank, pawnRank;
    board->pieceSets[(int)color] = set;
    if(color==WHITE){
        backRank = 0;
        pawnRank = 1;
    }
    else{
        assert(color==BLACK);
        backRank = 7;
        pawnRank = 6;
    }
    //order of back rank
    pieceType_e backRankOrder[] = {
        ROOK, KNIGHT, BISHOP, QUEEN, 
        KING, BISHOP, KNIGHT, ROOK
    };
    //init all the pieces
    for(i = 0; i<8; i++){
        ChessPieceSet_add(board, set, 
            ChessPiece_new(color, backRankOrder[i]),
            RANK_FILE(backRank,i));
    }
    for(i = 0; i < 8; i++){
        ChessPieceSet_add(board, set, 
            ChessPiece_new(color, PAWN),
            board, RANK_FILE(pawnRank,i));
    }
}
ChessPieceSet* ChessPieceSet_new(){
    ChessPieceSet* self = (ChessPieceSet*)malloc(sizeof(ChessPieceSet));
    int i,maxCount;
    //start with an empty set
    for(i = 0; i<6; i++){
        self->piecesCounts[i] = 0;
        if(i==TYPE_TO_INT(KING))
            maxCount=1;
        else if(i==TYPE_TO_INT(PAWN))
            maxCount=8;
        else
            maxCount=10;
        self->piecesByType[i] = (ChessPiece**)malloc(
            sizeof(ChessPiece*)*maxCount);
    }
    return self;
}
void ChessPieceSet_delete(ChessPieceSet* self){
    int i;
    for(i=0; i<6; i++)
        free(self->piecesByType[i]);
    free(self);
}

void __addPiece(ChessBoard* board, ChessPieceSet* set, 
    ChessPiece* piece, location_t loc){
    assert(piece!=NULL);
    assert(board!=NULL);
    assert(set!=NULL);
    assert(loc!=UNKNOWN_LOCATION);
    int type = TYPE_TO_INT(piece->type);
    int* count = (set->piecesCounts[type])&;
    self->piecesByType[(*count)++] = piece;
}
void __removePiece(ChessBoard* board, ChessPieceSet* set,
    ChessPiece* piece){
    int type = TYPE_TO_INT(piece->type);
    int* count = (set->piecesCounts[type])&;
    int i;
    for(i=0; set->piecesByType[type]!=piece; i++);
    for(i++;i<*count;i++)
        set->piecesByCounts[i-1]=set->piecesByCounts[i];
}

ChessBoard* ChessBoard_new(){
    ChessBoard* board = (ChessBoard*)malloc(sizeof(ChessBoard));
    int i;
    for(i=0; i<64; i++)
        board->squares[i]=NULL;
    board->hash = 0;
    board->hash = board->hash
        ^ZOBRIST_TABLE[ZOB_WHITE_KING_CASTLE]
        ^ZOBRIST_TABLE[ZOB_WHITE_QUEEN_CASTLE]
        ^ZOBRIST_TABLE[ZOB_BLACK_KING_CASTLE]
        ^ZOBRIST_TABLE[ZOB_BLACK_QUEEN_CASTLE];
    board->flags = 0;
    board->flags = board->flags
        ^WHITE_KING_CASTLE_FLAG
        ^WHITE_QUEEN_CASTLE_FLAG
        ^BLACK_KING_CASTLE_FLAG
        ^BLACK_QUEEN_CASTLE_FLAG;
    board->previous = NULL;
    board->next = NULL;
    board->nextCount = -1;
    board->whitePieces=NULL;
    board->blackPieces=NULL;
    ChessPieceSet_initSide(board, WHITE);
    ChessPieceSet_initSide(board, BLACK);
    return board;
}
ChessBoard* ChessBoard_clone(ChessBoard* self){
    ChessBoard* clone = (ChessBoard*)malloc(sizeof(ChessBoard));
    clone->flags = self->flags;
    clone->hash = self->hash;
    int i;
    for(i = 0; i < 64; i++){
        if(self->squares[i]==NULL)
            clone->squares[i] = NULL;
        else
            clone->squares[i] = _ChessPiece_clone(self->squares[i]);
    }
    _ChessPieceSet_clone(self->whitePieces, clone);
    _ChessPieceSet_clone(self->blackPieces, clone);
    clone->previous = NULL;
    clone->next = NULL;
    clone->nextCount = -1;
    return clone;
}
void ChessBoard_delete(ChessBoard* self){
    assert(self->next==NULL);
    ChessPieceSet_delete(self->whitePieces);
    ChessPieceSet_delete(self->blackPieces);
    free(self);
}
void ChessBoard_deleteAllNext(ChessBoard* self){
    int i;
    for(i = 0; i < self->nextCount; i++){
        ChessBoard_deleteAllNext(self->next[i]);
        ChessBoard_delete(self->next[i]);
    }
    if(self->next!=NULL)
        free(self->next);
    self->next = NULL;
    self->nextCount = -1;
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
    assert(loc<64);
    assert(self->squares[loc]==NULL);
    self->squares[loc] = piece;
    piece->location = loc;
    zob_hash_t hashAdd = ZOBRIST_TABLE[ChessPiece_getZobristID(piece)];
    self->hash = self->hash^hashAdd;
}
void ChessBoard_removePiece(ChessBoard* self, ChessPiece* piece){
    assert(piece->location>=0);
    assert(piece->location<64);
    assert(self->squares[piece->location]==piece);
    zob_hash_t hashSub = ZOBRIST_TABLE[ChessPiece_getZobristID(piece)];
    self->squares[piece->location] = NULL;
    piece->location = UNKNOWN_LOCATION;
    self->hash = self->hash^hashSub;
}
void ChessBoard_movePiece(ChessBoard* self, ChessPiece* piece,
    location_t to){
    assert(self->squares[to]==NULL);
    zob_hash_t hashSub = ZOBRIST_TABLE[ChessPiece_getZobristID(piece)];
    self->squares[piece->location] = NULL;
    piece->location = to;
    self->squares[to] = piece;
    zob_hash_t hashAdd = ZOBRIST_TABLE[ChessPiece_getZobristID(piece)];
    self->hash = self->hash^hashSub^hashAdd;
}
void ChessBoard_movePieceByLoc(ChessBoard* self, location_t from,
    location_t to){
    ChessPiece* toMove = self->squares[from];
    ChessBoard_movePiece(self, toMove, to);
}
void ChessBoard_toggleToPlay(ChessBoard* self){
    self->flags = self->flags^TO_PLAY_FLAG;
    zob_hash_t hash = ZOBRIST_TABLE[ZOB_TO_PLAY];
    self->hash = self->hash^hash;
}
void ChessBoard_unsetCastleFlag(ChessBoard* self, flag_t flag){
    if(self->flags&flag){
        self->flags = self->flags&(~flag);
        int zob_id;
        switch(flag){
        case WHITE_KING_CASTLE_FLAG:
            zob_id=ZOB_WHITE_KING_CASTLE;
            break;
        case WHITE_QUEEN_CASTLE_FLAG:
            zob_id=ZOB_WHITE_QUEEN_CASTLE;
            break;
        case BLACK_KING_CASTLE_FLAG:
            zob_id=ZOB_BLACK_KING_CASTLE;
            break;
        case BLACK_QUEEN_CASTLE_FLAG:
            zob_id=ZOB_BLACK_QUEEN_CASTLE;
            break;
        default:
            assert(0);
        }
        zob_hash_t hash = ZOBRIST_TABLE[zob_id];
        self->hash = self->hash^hash;
    }
}
void ChessBoard_setEnPassantFlags(ChessBoard* self, int file){
    ChessBoard_clearEnPassantFlags(self);
    self->flags = self->flags|(file<<EN_PASSANT_FILE_OFFSET)|
        EN_PASSANT_FLAG;
    zob_hash_t hash = ZOBRIST_TABLE[file+ZOB_EN_PASSANT_START];
    self->hash^=hash;
}
void ChessBoard_clearEnPassantFlags(ChessBoard* self){
    if(self->flags&EN_PASSANT_FLAG){
        int file = (self->flags>>EN_PASSANT_FILE_OFFSET)&7;
        zob_hash_t hash = ZOBRIST_TABLE[file+ZOB_EN_PASSANT_START];
        self->hash^=hash;
        self->flags&=~EN_PASSANT_FLAG;
        self->flags&=~(7<<EN_PASSANT_FILE_OFFSET);
    }
}
void ChessBoard_quickRemoveByLoc(ChessBoard* self, location_t loc){
    ChessPiece* piece = self->squares[loc];
    ChessPieceSet* set = piece->color==WHITE?
        self->whitePieces:self->blackPieces;
    ChessPieceSet_remove(set, piece, self);
    ChessPiece_delete(piece);
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
                    _ChessBoard_getChar(self, RANK_FILE(r,f)));
            }
        }
        printf("\n");
    }
}
