#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include "board.h"
#include "zobrist.h"

int main(void){
    initZobrist();
    printf("%d\n", (int)ZOBRIST_TABLE[10]);
    ChessBoard* board = ChessBoard_new();
    ChessBoard_print(board);
    ChessBoard_delete(board);
    closeZobrist();
    return 0;
}
