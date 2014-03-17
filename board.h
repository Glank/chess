#ifndef CHESS_BOARD_H_INCLUDE
#define CHESS_BOARD_H_INCLUDE
#include <inttypes.h>
typedef struct ChessPiece ChessPiece;
typedef struct ChessPieceSet ChessPieceSet;
typedef struct ChessPieceSetNode ChessPieceSetNode;
typedef struct ChessBoard ChessBoard;
typedef uint8_t location_t;
#define UNKNOWN_LOCATION (255)
#define RANK_FILE(r,f) ((r<<3)|f)
#define GET_RANK(loc) (loc>>3)
#define GET_FILE(loc) (loc&7)
typedef enum {WHITE, BLACK} color_e;
typedef enum {KING, QUEEN, ROOK, KNIGHT, BISHOP, PAWN} pieceType_e;

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
    ChessPiece* king;
    ChessPiece** queens;
    int queensCount;
    ChessPiece** rooks;
    int rooksCount;
    ChessPiece** knights;
    int knightsCount;
    ChessPiece** bishops;
    int bishopsCount;
    ChessPiece** pawns;
    int pawnsCount;
};
//create new sets and add them to the board
// that includes putting the pieces on the correct squares
void ChessPieceSet_initWhite(ChessBoard* board);
void ChessPieceSet_initBlack(ChessBoard* board);
//frees the set and *all of it's pieces*
void ChessPieceSet_delete(ChessPieceSet* set);

struct ChessBoard{
    ChessBoard* previous;
    ChessBoard** next;
    ChessPiece* squares[64];
    ChessPieceSet* whitePieces;
    ChessPieceSet* blackPieces;
};
ChessBoard* ChessBoard_new();
//frees everything except previous and next
void ChessBoard_delete(ChessBoard* self);
void ChessBoard_setPiece(ChessBoard* self, 
    ChessPiece* piece, location_t loc
);
ChessBoard* ChessBoard_clone(ChessBoard* self);
void ChessBoard_print(ChessBoard* self);
#endif
