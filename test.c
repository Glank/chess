#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include "board.h"
#include "zobrist.h"
#include "moves.h"
#define LOC(str) ((str[1]-'1')<<3)+((str[0]-'a'))

int main(void){
    initZobrist();
    ChessBoard* board = ChessBoard_new();
    ChessMoveGenerator* gen = ChessMoveGenerator_new();

    printf("%d\n", (int)board->hash);
    ChessBoard_print(board);

    ChessMoveGenerator_generateMoves(gen, board);
    printf("%d\n", board->nextCount);

    /*
    int i;
    ChessBoard* next;
    for(i = 0; i < board->nextCount; i++){
        next = board->next[i];
        ChessBoard_print(next);
        printf("\n");
    }
    //*/

    ChessMoveGenerator_delete(gen);
    ChessBoard_deleteAllNext(board);
    ChessBoard_delete(board);
    closeZobrist();
    return 0;
}
