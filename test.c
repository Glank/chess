#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include "board.h"
#include "zobrist.h"
#include "moves.h"
#define LOC(str) (((str[1]-'1')<<3)+((str[0]-'a')))

/**
 * See:
 * http://chessprogramming.wikispaces.com/Perft
 * http://chessprogramming.wikispaces.com/Perft+Results
 **/
unsigned long perft(ChessBoard* start, ChessMoveGenerator* gen, 
    int depth){
    if(depth==0)
        return 1;

    int i;
    unsigned long nodes = 0;
    move_t *next = NULL;
    int nextCount = 0;
    ChessMoveGenerator_generateMoves(gen, &next, &nextCount);
    for (i = 0; i < nextCount; i++){
        ChessBoard_makeMove(start, next[i]);
        nodes += perft(start, gen, depth-1);
        ChessBoard_unmakeMove(start);
    }
    free(next);
    return nodes;
}

int main(void){
    initZobrist();
    ChessBoard* board = ChessBoard_new();
    ChessBoard_setUp(board);
    ChessMoveGenerator* gen = ChessMoveGenerator_new(board);

/*
    move_t move = NEW_MOVE(LOC("d2"),LOC("d5"));
    ChessBoard_makeMove(board, move);
    move = NEW_MOVE(LOC("e7"),LOC("e5"));
    move|=DOUBLE_PAWN_PUSH_MOVE;
    ChessBoard_makeMove(board, move);
    move = NEW_MOVE(LOC("d3"),LOC("d4"));
    ChessBoard_makeMove(board, move);
    move = NEW_MOVE(LOC("e5"),LOC("d4"));
    move|=CAPTURE_MOVE_FLAG;
    ChessBoard_makeMove(board, move);
    move = NEW_MOVE(LOC("e2"),LOC("e4"));
    move|=DOUBLE_PAWN_PUSH_MOVE;
    ChessBoard_makeMove(board, move);
    move = NEW_MOVE(LOC("d4"),LOC("e3"));
    move|=EN_PASSANT_MOVE;
    ChessBoard_makeMove(board, move);
    move = NEW_MOVE(LOC("d1"),LOC("d7"));
    move|=CAPTURE_MOVE_FLAG;
    ChessBoard_makeMove(board, move);
*/
    
    ChessBoard_print(board);

    printf("%lu\n", perft(board, gen, 5));
/*
    int i;
    for(i = 0; i < nextCount; i++){
        printf("\n%x\n", next[i]);
        ChessBoard_makeMove(board, next[i]);
        ChessBoard_print(board);
        ChessBoard_unmakeMove(board);
    }
*/

    ChessMoveGenerator_delete(gen);
    ChessBoard_delete(board);
    closeZobrist();
    return 0;
}
