#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "board.h"


void __addPiece(ChessBoard* board, ChessPiece* piece);
void __movePieceByLoc(ChessBoard* self, location_t from,
    location_t to);
void __setEnPassantFlags(ChessBoard* self, int file);
void __clearEnPassantFlags(ChessBoard* self);
void __unsetCastleFlag(ChessBoard* self, flag_t flag);
void __toggleToPlay(ChessBoard* self);

ChessPiece* ChessPiece_new(color_e color, pieceType_e type,
    location_t location){
    ChessPiece* self = (ChessPiece*)malloc(sizeof(ChessPiece));
    self->location = location;
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
//set up one side of the board and it's ChessPieceSet
void __initSide(ChessBoard* board, color_e color){
    ChessPieceSet* set = board->pieceSets[(int)color];
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
    int i;
    for(i = 0; i<8; i++){
        __addPiece(board, ChessPiece_new(
            color, backRankOrder[i], RANK_FILE(backRank,i)));
        __addPiece(board, ChessPiece_new(
            color, PAWN, RANK_FILE(pawnRank,i)));
    }
}
ChessPieceSet* ChessPieceSet_new(){
    ChessPieceSet* self = (ChessPieceSet*)malloc(
        sizeof(ChessPieceSet));
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

void __addPiece(ChessBoard* board, ChessPiece* piece){
    assert(piece!=NULL);
    assert(board!=NULL);
    ChessPieceSet* set = board->pieceSets[(int)(piece->color)];
    assert(set!=NULL);
    int type = TYPE_TO_INT(piece->type);
    int* count = &(set->piecesCounts[type]);
    set->piecesByType[TYPE_TO_INT(piece->type)][(*count)++] = piece;
    assert(board->squares[piece->location]==NULL);
    board->squares[piece->location] = piece;
    zob_hash_t hash = ZOBRIST_TABLE[__getZobristID(piece)];
    board->hash^=hash;
}
void __capturePiece(ChessBoard* board, ChessPiece* piece){
    ChessPieceSet* set = board->pieceSets[(int)(piece->color)];
    int type = TYPE_TO_INT(piece->type);
    int* count = &(set->piecesCounts[type]);
    int i;
    for(i=0; set->piecesByType[type][i]!=piece; i++);
    for(i++;i<*count;i++)
        set->piecesByType[type][i-1]=set->piecesByType[type][i];
    assert(board->squares[piece->location]==piece);
    board->squares[piece->location] = NULL;
    assert(board->capturedCount<MAX_CAPTURES);
    board->captured[board->capturedCount] = piece;
    board->capturedCount++;
    zob_hash_t hash = ZOBRIST_TABLE[__getZobristID(piece)];
    board->hash^=hash;
}

ChessBoard* ChessBoard_new(){
    ChessBoard* board = (ChessBoard*)malloc(sizeof(ChessBoard));
    board->movesCount = 0;
    board->capturedCount = 0;
    int i;
    for(i=0; i<64; i++)
        board->squares[i]=NULL;
    board->pieceSets[(int)WHITE] = ChessPieceSet_new();
    board->pieceSets[(int)BLACK] = ChessPieceSet_new();
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
    return board;
}
void ChessBoard_setUp(ChessBoard* self){
    __initSide(self, WHITE);
    __initSide(self, BLACK);
}
void ChessBoard_delete(ChessBoard* self){
    int i;
    //delete all captured pieces
    for(i=0; i<self->capturedCount; i++)
        ChessPiece_delete(self->captured[i]);
    //delete all pieces left on the squares
    for(i=0; i<64; i++){
        if(self->squares[i]!=NULL)
            ChessPiece_delete(self->squares[i]);
    }
    //free the piece sets
    ChessPieceSet_delete(self->pieceSets[0]);
    ChessPieceSet_delete(self->pieceSets[1]);
    //free everything else
    free(self);
}
void ChessBoard_makeMove(ChessBoard* self, move_t move){
    //push the move onto the move stack
    self->prevHashes[self->movesCount] = self->hash;
    self->prevFlags[self->movesCount] = self->flags;
    self->moves[self->movesCount++] = move;
    __toggleToPlay(self);
    __clearEnPassantFlags(self);
    int meta = GET_META(move);
    location_t from = GET_FROM(move);
    location_t to = GET_TO(move);
    int rank, file;
    //do special moves
    switch(meta){
    case DOUBLE_PAWN_PUSH_MOVE:
        __setEnPassantFlags(self, GET_FILE(to));
        break;
    case KING_CASTLE_MOVE:
    case QUEEN_CASTLE_MOVE:
        rank = GET_RANK(from);
        if(meta==KING_CASTLE_MOVE)
            __movePieceByLoc(self, 
                RANK_FILE(rank, 7), RANK_FILE(rank, 5));
        else
            __movePieceByLoc(self, 
                RANK_FILE(rank, 0), RANK_FILE(rank, 3));
        if(rank==0){//white
            __unsetCastleFlag(self, WHITE_KING_CASTLE_FLAG);
            __unsetCastleFlag(self, WHITE_QUEEN_CASTLE_FLAG);
        }
        else{//black
            __unsetCastleFlag(self, BLACK_KING_CASTLE_FLAG);
            __unsetCastleFlag(self, BLACK_QUEEN_CASTLE_FLAG);
        }
        break;
    case EN_PASSANT_MOVE:
        rank = GET_RANK(from);
        file = GET_FILE(to);
        __capturePiece(self, 
            self->squares[RANK_FILE(rank,file)]);
        break;
    default:
        if(meta&CAPTURE_MOVE_FLAG)
            __capturePiece(self, self->squares[to]);
        if(meta&PROMOTION_MOVE_FLAG){
            ChessPiece* piece = self->squares[from];
            switch(meta&PROMOTION_MASK){
            case QUEEN_PROMOTION:
                piece->type=QUEEN;
                break;
            case ROOK_PROMOTION:
                piece->type=ROOK;
                break;
            case KNIGHT_PROMOTION:
                piece->type=KNIGHT;
                break;
            case BISHOP_PROMOTION:
                piece->type=BISHOP;
                break;
            default:
                assert(0);
            }
        }
    }
    __movePieceByLoc(self, GET_FROM(move), GET_TO(move));
}
char __getChar(ChessBoard* self, location_t loc){
    assert(self!=NULL);
    assert(0<=loc);
    assert(loc<64);
    ChessPiece* piece = self->squares[loc];
    if(piece==NULL)
        return ((GET_RANK(loc)^GET_FILE(loc))%2)?'+':'#';
    return __getPieceChar(piece);
}
void __movePiece(ChessBoard* self, ChessPiece* piece,
    location_t to){
    assert(self->squares[to]==NULL);
    zob_hash_t hashSub = ZOBRIST_TABLE[__getZobristID(piece)];
    self->squares[piece->location] = NULL;
    piece->location = to;
    self->squares[to] = piece;
    zob_hash_t hashAdd = ZOBRIST_TABLE[__getZobristID(piece)];
    self->hash = self->hash^hashSub^hashAdd;
}
void __movePieceByLoc(ChessBoard* self, location_t from,
    location_t to){
    ChessPiece* toMove = self->squares[from];
    __movePiece(self, toMove, to);
}
void __toggleToPlay(ChessBoard* self){
    self->flags = self->flags^TO_PLAY_FLAG;
    zob_hash_t hash = ZOBRIST_TABLE[ZOB_TO_PLAY];
    self->hash = self->hash^hash;
}
void __unsetCastleFlag(ChessBoard* self, flag_t flag){
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
void __clearEnPassantFlags(ChessBoard* self){
    if(self->flags&EN_PASSANT_FLAG){
        int file = (self->flags>>EN_PASSANT_FILE_OFFSET)&7;
        zob_hash_t hash = ZOBRIST_TABLE[file+ZOB_EN_PASSANT_START];
        self->hash^=hash;
        self->flags&=~EN_PASSANT_FLAG;
        self->flags&=~(7<<EN_PASSANT_FILE_OFFSET);
    }
}
void __setEnPassantFlags(ChessBoard* self, int file){
    assert(!(self->flags&EN_PASSANT_FLAG));
    self->flags = self->flags|(file<<EN_PASSANT_FILE_OFFSET)|
        EN_PASSANT_FLAG;
    zob_hash_t hash = ZOBRIST_TABLE[file+ZOB_EN_PASSANT_START];
    self->hash^=hash;
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
                printf("%c ", __getChar(self, RANK_FILE(r,f)));
            }
        }
        printf("\n");
    }
    printf("Castle Flags: ");
    if(self->flags&WHITE_KING_CASTLE_FLAG)
        printf("K");
    if(self->flags&WHITE_QUEEN_CASTLE_FLAG)
        printf("Q");
    if(self->flags&BLACK_KING_CASTLE_FLAG)
        printf("k");
    if(self->flags&BLACK_QUEEN_CASTLE_FLAG)
        printf("q");
    printf("\n");
    if(!(self->flags&EN_PASSANT_FLAG)){
        printf("No en passant flags set.\n");
        int file = 7&(self->flags>>EN_PASSANT_FILE_OFFSET);
        assert(file==0);
    }
    else{
        int file = 7&(self->flags>>EN_PASSANT_FILE_OFFSET);
        printf("En passant flags set '%c' file.\n",
            (char)('a'+file));
    }
    printf("Flags: %x\n", self->flags);
    printf("Hash: %x\n", self->hash);
    printf("Moves:\n");
    int i;
    for(i = 0; i < self->movesCount; i++){
        printf("%d)\t", i);
        printf("Move:%x\t", self->moves[i]);
        printf("Prev Flags:%x\t", self->prevFlags[i]);
        printf("Prev Hash:%x\n",  self->prevHashes[i]);
    }
    printf("Captured Stack:\n");
    for(i = 0; i < self->capturedCount; i++){
        printf("%d)\t%c\n",i,
            __getPieceChar(self->captured[i])); 
    }
}
