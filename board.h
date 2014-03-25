#ifndef CHESS_BOARD_H_INCLUDE
#define CHESS_BOARD_H_INCLUDE
#include <inttypes.h>
#include "zobrist.h"
typedef struct ChessPiece ChessPiece;
typedef struct ChessPieceSet ChessPieceSet;
typedef struct ChessBoard ChessBoard;
typedef uint8_t location_t;
#define UNKNOWN_LOCATION 255
#define RANK_FILE(r,f) (((r)<<3)|(f))
#define GET_RANK(loc) ((loc)>>3)
#define GET_FILE(loc) ((loc)&7)
typedef uint16_t flag_t;
#define WHITE_KING_CASTLE_FLAG 0x001
#define WHITE_QUEEN_CASTLE_FLAG 0x002
#define BLACK_KING_CASTLE_FLAG 0x004
#define BLACK_QUEEN_CASTLE_FLAG 0x008
#define EN_PASSANT_FLAG 0x010
#define EN_PASSANT_FILE_OFFSET 5
#define TO_PLAY_FLAG 0x400
#define WHITE_IN_CHECK_FLAG 0x100
#define BLACK_IN_CHECK_FLAG 0x200
typedef enum {WHITE=0, BLACK=1} color_e;
#define OTHER_COLOR(c) ((color_e)(((int)(c))^1))
typedef enum {
    KING=0, QUEEN=2, ROOK=4, KNIGHT=6, BISHOP=8, PAWN=10
} pieceType_e;
#define TYPE_TO_INT(type) (((int)(type))>>1)
#define TYPE_COLOR_TO_INT(type,color) (((int)(type))+((int)(color)))
typedef uint16_t move_t;
#define NEW_MOVE(from,to) ((from<<6)|to)
//encoding for a chess move is specified here:
//https://chessprogramming.wikispaces.com/Encoding+Moves
//first 2 bits are the capture and pawn promotion flags,
//the next 2 bits are used in conjunction with the first two
//to identify the special move type,
//next 6 are the from location,
//last 6 are the to location
#define CAPTURE_MOVE_FLAG 0x4000
#define PROMOTION_MOVE_FLAG 0x8000
//special moves
#define DOUBLE_PAWN_PUSH_MOVE 0x1000
#define KING_CASTLE_MOVE 0x2000
#define QUEEN_CASTLE_MOVE 0x3000
#define EN_PASSANT_MOVE 0x5000
//promotion types (must recognize flags first)
#define KNIGHT_PROMOTION 0x0000
#define BISHOP_PROMOTION 0x1000
#define ROOK_PROMOTION 0x2000
#define QUEEN_PROMOTION 0x3000

struct ChessPiece{
    location_t location;
    color_e color;
    pieceType_e type;
};

//This structure should act only as an index
//to help accelerate operations done by ChessBoard.
//It should not be interfaced with by anything outside of
//board.c
struct ChessPieceSet{
    ChessPiece** piecesByType[6];
    int piecesCounts[6];
};

struct ChessBoard{
    move_t* moves;
    int movesCount;
    ChessPiece* captured[30];
    int capturedCount;
    ChessPiece* squares[64];
    ChessPieceSet** pieceSets;
    flag_t flags;
    zob_hash_t hash;
};
ChessBoard* ChessBoard_new();
void ChessBoard_delete(ChessBoard* self);
void ChessBoard_setUp(ChessBoard* self);
void ChessBoard_makeMove(ChessBoard* self, move_t move);
void ChessBoard_unmakeMove(ChessBoard* self);
void ChessBoard_print(ChessBoard* self);
#endif
