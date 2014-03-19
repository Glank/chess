#ifndef CHESS_BOARD_H_INCLUDE
#define CHESS_BOARD_H_INCLUDE
#include <inttypes.h>
#include "zobrist.h"
typedef struct ChessPiece ChessPiece;
typedef struct ChessPieceSet ChessPieceSet;
typedef struct ChessPieceSetNode ChessPieceSetNode;
typedef struct ChessBoard ChessBoard;
typedef uint8_t location_t;
#define UNKNOWN_LOCATION 255
#define RANK_FILE(r,f) (((r)<<3)|(f))
#define GET_RANK(loc) ((loc)>>3)
#define GET_FILE(loc) ((loc)&7)
typedef uint16_t flag_t;
#define WHITE_KING_CASTLE_FLAG 1
#define WHITE_QUEEN_CASTLE_FLAG 2
#define BLACK_KING_CASTLE_FLAG 4
#define BLACK_QUEEN_CASTLE_FLAG 8
#define EN_PASSANT_FLAGS_OFFSET 4
#define TO_PLAY_FLAG 128
#define WHITE_IN_CHECK_FLAG 256
#define BLACK_IN_CHECK_FLAG 512
typedef enum {WHITE=0, BLACK=1} color_e;
#define OTHER_COLOR(c) ((color_e)(((int)(c))^1))
typedef enum {
    KING=0, QUEEN=2, ROOK=4, KNIGHT=6, BISHOP=8, PAWN=10
} pieceType_e;
#define TYPE_TO_INT(type) (((int)(type))>>1)
#define TYPE_COLOR_TO_INT(type,color) (((int)(type))+((int)(color)))

struct ChessPiece{
    location_t location;
    color_e color;
    pieceType_e type;
};
ChessPiece* ChessPiece_new(
    color_e color,
    pieceType_e type
);
//warning, does not remove from square
void ChessPiece_delete(ChessPiece* self);
char ChessPiece_getChar(ChessPiece* self);
int ChessPiece_getZobristID(ChessPiece* self);

struct ChessPieceSet{
    ChessPiece** piecesByType[6];
    int piecesCounts[6];
};
//create new sets and add them to the board
// that includes putting the pieces on the correct squares
void ChessPieceSet_initSide(ChessBoard* board, color_e color);
//adds a piece to the set and the board
void ChessPieceSet_add(ChessPieceSet* self, ChessPiece* piece,
    ChessBoard* board, location_t loc);
//removes a piece from the set and the board but does not free it.
void ChessPieceSet_remove(ChessPieceSet* self, ChessPiece* piece,
    ChessBoard* board);
//frees the set and *all of it's pieces*
void ChessPieceSet_delete(ChessPieceSet* set);

struct ChessBoard{
    ChessBoard* previous;
    ChessBoard** next;
    int nextCount;
    ChessPiece* squares[64];
    ChessPieceSet* whitePieces;
    ChessPieceSet* blackPieces;
    flag_t flags;
    zob_hash_t hash;
};
ChessBoard* ChessBoard_new();
//frees everything except previous and next
void ChessBoard_delete(ChessBoard* self);
void ChessBoard_setPiece(ChessBoard* self, 
    ChessPiece* piece, location_t loc);
void ChessBoard_removePiece(ChessBoard* self, ChessPiece* piece);
void ChessBoard_movePiece(ChessBoard* self, ChessPiece* piece,
    location_t loc);
void ChessBoard_movePieceByLoc(ChessBoard* self, location_t from,
    location_t to);
ChessBoard* ChessBoard_clone(ChessBoard* self);
void ChessBoard_toggleToPlay(ChessBoard* self);
void ChessBoard_unsetCastleFlag(ChessBoard* self, flag_t flag);
void ChessBoard_setEnPassantFlags(ChessBoard* self, int file);
void ChessBoard_clearEnPassantFlags(ChessBoard* self);
void ChessBoard_print(ChessBoard* self);
#endif
