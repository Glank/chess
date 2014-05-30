#ifndef CHESS_BOARD_H_INCLUDE
#define CHESS_BOARD_H_INCLUDE
#include <inttypes.h>
#include "zobrist.h"
#include "strutl.h"
#include "move.h"
typedef struct ChessPiece ChessPiece;
typedef struct ChessPieceSet ChessPieceSet;
typedef struct ChessBoard ChessBoard;
typedef struct BoardBackup BoardBackup;
typedef struct GameInfo GameInfo;
typedef uint16_t flag_t;
#define WHITE_KING_CASTLE_FLAG 0x001
#define WHITE_QUEEN_CASTLE_FLAG 0x002
#define BLACK_KING_CASTLE_FLAG 0x004
#define BLACK_QUEEN_CASTLE_FLAG 0x008
#define EN_PASSANT_FLAG 0x010
#define EN_PASSANT_FILE_OFFSET 5
#define TO_PLAY_FLAG 0x400
typedef enum {WHITE=0, BLACK=1} color_e;
#define OTHER_COLOR(c) ((color_e)(((int)(c))^1))
typedef enum {
    KING=0, QUEEN=2, ROOK=4, KNIGHT=6, BISHOP=8, PAWN=10
} pieceType_e;
#define KING_INDEX 0
#define QUEEN_INDEX 1
#define ROOK_INDEX 2
#define KNIGHT_INDEX 3
#define BISHOP_INDEX 4
#define PAWN_INDEX 5
#define TYPE_TO_INT(type) (((int)(type))>>1)
#define TYPE_COLOR_TO_INT(type,color) (((int)(type))+((int)(color)))
//some max constants arbitrarily set
#define MAX_MOVES 1024
#define MAX_CAPTURES 30
//some important locations
#define WHITE_QUEEN_ROOK_START 0
#define WHITE_KING_START 4
#define WHITE_KING_ROOK_START 7
#define BLACK_QUEEN_ROOK_START 56
#define BLACK_KING_START 60
#define BLACK_KING_ROOK_START 63
//the FEN starting position
#define FEN_START "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 0"

//TODO: makes this read-only
struct ChessPiece{
    location_t location;
    color_e color;
    pieceType_e type;
};

//TODO: make this private
struct ChessPieceSet{
    ChessPiece** piecesByType[6];
    int piecesCounts[6];
};

//TODO: make this private
struct BoardBackup{
    flag_t flags;
    zob_hash_t hash;
    int fiftyMoveCount;
};

//TODO: move this back into ChessBoard
struct GameInfo{
    move_t moves[MAX_MOVES];
    BoardBackup backups[MAX_MOVES];
    ChessPiece* captured[MAX_CAPTURES];
    ChessPieceSet* pieceSets[2];
    int movesCount;
    int capturedCount;
};

//TODO: make this private
struct ChessBoard{
    int standard;
    void* extra;
    int fiftyMoveCount;
    ChessPiece* squares[64];
    flag_t flags;
    zob_hash_t hash;
};
ChessBoard* ChessBoard_new(const char* fen);
void ChessBoard_delete(ChessBoard* self);
int ChessBoard_equals(ChessBoard* self, ChessBoard* other);
ChessBoard* ChessBoard_copy(ChessBoard* self);
//implementation of this method is found in moves.c
int ChessBoard_testForCheck(ChessBoard* board);
int ChessBoard_isInOptionalDraw(ChessBoard* board);
void ChessBoard_makeMove(ChessBoard* self, move_t move);
move_t ChessBoard_unmakeMove(ChessBoard* self);
void ChessBoard_print(ChessBoard* self);
void ChessBoard_longPrint(ChessBoard* self);
int ChessBoard_testForCheckmate(ChessBoard* self);
int ChessBoard_testForStalemate(ChessBoard* self);
#endif
