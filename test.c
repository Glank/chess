#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include "board.h"
#include "zobrist.h"

int main(void){
    initZobrist();
    ChessBoard* board = ChessBoard_new();
    ChessBoard* clone = ChessBoard_clone(board);
    printf("%d\n", (int)clone->hash);

    ChessPiece* kingPawn = clone->squares[RANK_FILE(1,4)];
    ChessBoard_movePiece(clone, kingPawn, RANK_FILE(3,4));

    ChessBoard_print(clone);
    printf("\n");
    ChessBoard_print(board);
    
    ChessBoard_delete(board);
    ChessBoard_delete(clone);
    closeZobrist();
    return 0;
}
