#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include "board.h"
#include "zobrist.h"
#include "moves.h"
#define LOC(str) ((str[1]-'1')<<3)+((str[0]-'a'))

unsigned long perft(ChessBoard* start, ChessMoveGenerator* gen, 
    int depth){
    int i;
    unsigned long nodes = 0;
    
    ChessMoveGenerator_generateMoves(gen, start);
    if(start->nextCount==0){
        if(start->flags&TO_PLAY_FLAG && 
            start->flags&WHITE_IN_CHECK_FLAG)
            printf("Checkmate");
        if(!(start->flags&TO_PLAY_FLAG) && 
            start->flags&BLACK_IN_CHECK_FLAG)
            printf("Checkmate");
    }
    if(depth==0){
        ChessBoard_deleteAllNext(start);
        return 1;
    }
    for (i = 0; i < start->nextCount; i++)
        nodes += perft(start->next[i], gen, depth-1);
    ChessBoard_deleteAllNext(start);
    return nodes;
}

int main(void){
    initZobrist();
    ChessBoard* board = ChessBoard_new();
    ChessMoveGenerator* gen = ChessMoveGenerator_new();

    printf("%d\n", (int)board->hash);
    ChessBoard_print(board);

    printf("%d\n", perft(board, gen, 3));

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
