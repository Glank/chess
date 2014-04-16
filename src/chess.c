#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <assert.h>
#include <string.h>
#include "board.h"
#include "zobrist.h"
#include "moves.h"
#include "heuristics.h"
#include "search.h"
#include "narrator.h"

int main(int argc, char** argv){
    initZobrist();
    ChessBoard* board = ChessBoard_new(argv[1]);
    initChessHeuristics(board);

    ChessBoard_print(board);
    
    int depth = 4;
    if(argc>2)
        depth = strtol(argv[2], NULL, 10);

    move_t line[MAX_LINE_LENGTH];
    int length;
    int eval = getBestLine(board, depth, line, &length);
    printf("%d\t", eval);
    int i;
    char moveOut[10];
    int moveOutSize;
    for(i = 0; i < length; i++){
        if(i!=0)
            printf(" ");
        toAlgebraicNotation(line[i], board, moveOut, &moveOutSize);
        printf("%s", moveOut);
        ChessBoard_makeMove(board, line[i]);
    }
    printf("\n");


    ChessBoard_delete(board);
    closeChessHeuristics();
    closeZobrist();
    return 0;
}
