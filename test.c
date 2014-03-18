#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include "board.h"
#include "zobrist.h"
#include "moves.h"

int main(void){
    initZobrist();
    ChessBoard* board = ChessBoard_new();
    ChessBoard* clone = ChessBoard_clone(board);

    ChessPiece* kingPawn = clone->squares[RANK_FILE(1,4)];
    ChessBoard_movePiece(clone, kingPawn, RANK_FILE(3,4));

    printf("%d\n", (int)board->hash);
    ChessBoard_print(board);
    printf("\n");
    printf("%d\n", (int)clone->hash);
    ChessBoard_print(clone);
    
    ChessBoard_delete(board);
    ChessBoard_delete(clone);
    closeZobrist();
    return 0;
}
