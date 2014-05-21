#ifndef CHESS_GAME_H_INCLUDE
#define CHESS_GAME_H_INCLUDE
#include "mind.h"
#include "pgn.h"
typedef struct ChessGame ChessGame;
ChessGame* ChessGame_new();
void ChessGame_delete(ChessGame* self);
void ChessGame_setFEN(ChessGame* self, char* fen);
void ChessGame_setHuman(ChessGame* self, int player, int human);
void ChessGame_setTimeout(ChessGame* self, int player, int timeout);
void ChessGame_play(ChessGame* self);
#endif
