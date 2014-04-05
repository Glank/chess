#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <assert.h>
#include "board.h"
#include "zobrist.h"
#include "moves.h"
#define LOC(str) (((str[1]-'1')<<3)+((str[0]-'a')))

/**
 * See:
 * http://chessprogramming.wikispaces.com/Perft
 * http://chessprogramming.wikispaces.com/Perft+Results
 **/
int checks = 0, captures = 0;
int checkmates = 0, ep = 0;
int promotions = 0;
int toCounts[64];
unsigned long perft(ChessBoard* start, ChessMoveGenerator* gen, 
    int depth){
    move_t *next = NULL;
    int nextCount = 0;
    if(depth==0){
        if(ChessBoard_testForCheck(start)){
            printf("\n");
            ChessBoard_print(start);
            checks++;
            ChessMoveGenerator_generateMoves(gen, 1, NULL);
            if(gen->nextCount==0)
                checkmates++;
        }
        return 1;
    }

    int i;
    unsigned long nodes = 0;
    ChessMoveGenerator_generateMoves(gen, 
        ChessBoard_testForCheck(start), NULL);
    ChessMoveGenerator_copyMoves(gen,
        &next, &nextCount);
    for (i = 0; i < nextCount; i++){
        ChessBoard_makeMove(start, next[i]);
        if(depth==1){
            toCounts[GET_FROM(next[i])]++;
            if(next[i]&CAPTURE_MOVE_FLAG)
                captures++;
            if(next[i]&PROMOTION_MOVE_FLAG)
                promotions++;
            if(GET_META(next[i])==EN_PASSANT_MOVE)
                ep++;
        }
        nodes += perft(start, gen, depth-1);
        ChessBoard_unmakeMove(start);
    }
    free(next);
    return nodes;
}


int main(void){
    initZobrist();
    ChessBoard* board = ChessBoard_new("r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1");
    ChessMoveGenerator* gen = ChessMoveGenerator_new(board);
    int i;
    for(i=0;i<64;i++)
        toCounts[i]=0;

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
    printf("perft: %lu\n", perft(board, gen, 3));
    printf("checks: %d\n", checks);
    printf("captures: %d\n", captures);
    printf("checkmates: %d\n", checkmates);
    printf("en passants: %d\n", ep);
    printf("promotions: %d\n", promotions);
    /*
    for(i=0;i<64;i++){
        printf("%c%c: %d\n", 'a'+GET_FILE(i), '1'+GET_RANK(i),
            toCounts[i]);
    }//*/
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
