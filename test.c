#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include "board.h"
#include "zobrist.h"
#include "moves.h"

int main(void){
    initZobrist();
    ChessBoard* board = ChessBoard_new();
    ChessMoveGenerator* gen = ChessMoveGenerator_new();

    printf("%d\n", (int)board->hash);
    ChessBoard_print(board);

    ChessMoveGenerator_generateMoves(gen, board);
    printf("%d\n", board->nextCount);

    ChessMoveGenerator_delete(gen);
    ChessBoard_deleteAllNext(board);
    ChessBoard_delete(board);
    closeZobrist();
    return 0;
}
