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
int checks = 0, captures = 0;
int checkmates = 0, ep = 0;
int toCounts[64];
unsigned long perft(ChessBoard* start, ChessMoveGenerator* gen, 
    int depth){
    move_t *next = NULL;
    int nextCount = 0;
    if(depth==0){
        if(ChessBoard_testForCheck(start)){
            checks++;
            ChessMoveGenerator_generateMoves(gen, &next, &nextCount);
            free(next);
            if(nextCount==0)
                checkmates++;
        }
        return 1;
    }

    int i;
    unsigned long nodes = 0;
    ChessMoveGenerator_generateMoves(gen, &next, &nextCount);
    for (i = 0; i < nextCount; i++){
        ChessBoard_makeMove(start, next[i]);
        nodes += perft(start, gen, depth-1);
        if(depth==1){
            toCounts[GET_FROM(next[i])]++;
            if(next[i]&CAPTURE_MOVE_FLAG)
                captures++;
            if(GET_META(next[i])==EN_PASSANT_MOVE)
                ep++;
        }
        ChessBoard_unmakeMove(start);
    }
    free(next);
    return nodes;
}

int main(void){
    initZobrist();
    ChessBoard* board = ChessBoard_new(FEN_START);
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

    printf("%lu\n", perft(board, gen, 4));
    printf("%d\n%d\n%d\n%d\n", checks, captures, checkmates, ep);
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
