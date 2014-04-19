#ifndef CHESS_NARRATOR_H_INCLUDE
#define CHESS_NARRATOR_H_INCLUDE
#include "board.h"
#include "moves.h"

//uses PNG Algebreic Notation
//these functions are slow - do not use for calculations
void toAlgebraicNotation(move_t move, ChessBoard* from, char* out, int* outSize);
move_t fromAlgebraicNotation(char* notation, ChessBoard* from);

#endif
