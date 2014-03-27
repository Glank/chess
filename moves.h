#ifndef CHESS_MOVES_H_INCLUDE
#define CHESS_MOVES_H_INCLUDE
#include "board.h"
//source for this constant:
//https://chessprogramming.wikispaces.com/Encoding+Moves#Various%20Encodings%20and%20Decorations-Move%20Index
#define MOVE_GEN_MAX_ALLOCATED 218
typedef struct ChessMoveGenerator ChessMoveGenerator;

struct ChessMoveGenerator{
    move_t next[MOVE_GEN_MAX_ALLOCATED];
    int nextCount;
    ChessBoard* currentBoard;
    color_e toPlay;
    int inCheck;
    ChessPieceSet* curSet;
};
//sets self->next and associated vars
//takes non-trivial time
ChessMoveGenerator* ChessMoveGenerator_new();
void ChessMoveGenerator_delete(ChessMoveGenerator* self);
void ChessMoveGenerator_generateMoves(
    ChessMoveGenerator* self, ChessBoard* from);
#endif
