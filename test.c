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
/*
unsigned long perft(ChessBoard* start, ChessMoveGenerator* gen, 
    int depth){
    int i;
    unsigned long nodes = 0;
    
    ChessMoveGenerator_generateMoves(gen, start);
    if(start->nextCount==0){
        if(start->flags&TO_PLAY_FLAG && 
            start->flags&WHITE_IN_CHECK_FLAG)
            printf("Checkmate\n");
        else if(!(start->flags&TO_PLAY_FLAG) && 
            start->flags&BLACK_IN_CHECK_FLAG)
            printf("Checkmate\n");
        else
            printf("Stalemate\n");
        ChessBoard_print(start);
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
*/

int main(void){
    initZobrist();
    ChessBoard* board = ChessBoard_new();
    ChessBoard_setUp(board);

    move_t move = NEW_MOVE(LOC("d2"),LOC("d3"));
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

    ChessBoard_print(board);

    //printf("%lu\n", perft(board, gen, 4));

    /*
    int i;
    ChessBoard* next;
    for(i = 0; i < board->nextCount; i++){
        next = board->next[i];
        ChessBoard_print(next);
        printf("\n");
    }
    //*/

    //ChessMoveGenerator_delete(gen);
    //ChessBoard_deleteAllNext(board);
    ChessBoard_delete(board);
    closeZobrist();
    return 0;
}
