#ifndef CHESS_BITBOARD_H_INCLUDE
#define CHESS_BITBOARD_H_INCLUDE
#include "zobrist.h"
#include "move.h"
typedef struct BitBoard BitBoard;
BitBoard* BitBoard_new();
//returns 0 if no error, 1 if error
void BitBoard_setUp(BitBoard* self, char* fen);
void BitBoard_delete(BitBoard* self);
void BitBoard_toString(BitBoard* self, char* out);
void BitBoard_print(BitBoard* self);

/* private:
void BitBoard_movePiece(BitBoard* self, int color, int type, location_t from, location_t to);
void BitBoard_togglePiece(BitBoard* self, int color, int type, location_t loc);
void BitBoard_toggleToPlay(BitBoard* self);
void BitBoard_toggleWhiteKingSideCastle(BitBoard* self);
void BitBoard_toggleWhiteQueenSideCastle(BitBoard* self);
void BitBoard_toggleBlackKingSideCastle(BitBoard* self);
void BitBoard_toggleBlackQueenSideCastle(BitBoard* self);
int BitBoard_canWhiteKingSideCastle(BitBoard* self);
int BitBoard_canWhiteQueenSideCastle(BitBoard* self);
int BitBoard_canBlackKingSideCastle(BitBoard* self);
int BitBoard_canBlackQueenSideCastle(BitBoard* self);
void BitBoard_setEnPassant(BitBoard* self, location_t loc);
void BitBoard_setHalfMoveClock(BitBoard* self, int value);
void BitBoard_setMoveNumber(BitBoard* self, int value);
//returns 1 if the bit board is valid, 0 if it is invalid
//if it is invalid, a global chess error is set (see chesserrors.h)
int BitBoard_validate(BitBoard* self);
*/

#endif
