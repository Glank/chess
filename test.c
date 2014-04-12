#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <assert.h>
#include "board.h"
#include "zobrist.h"
#include "moves.h"
#include "heuristics.h"
#include "search.h"
#include "narrator.h"
#define LOC(str) (((str[1]-'1')<<3)+((str[0]-'a')))
#define POS_1_PERFT3 8902
#define POS_2_PERFT3 97862
#define POS_3_PERFT3 2812
#define POS_4_PERFT3 9467
#define POS_5_PERFT3 53392
#define POS_6_PERFT3 89890
#define POS_2 "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq -"
#define POS_3 "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - -"
#define POS_4 "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1"
#define POS_5 "rnbqkb1r/pp1p1ppp/2p5/4P3/2B5/8/PPP1NnPP/RNBQK2R w KQkq - 0 6"
#define POS_6 "r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10"
const char* PERFT_STARTS[] = {FEN_START, POS_2, POS_3, POS_4, POS_5, POS_6};
const int PERFT3_VALUES[] = {POS_1_PERFT3, POS_2_PERFT3, POS_3_PERFT3, 
    POS_4_PERFT3, POS_5_PERFT3, POS_6_PERFT3};

/**
 * See:
 * http://chessprogramming.wikispaces.com/Perft
 * http://chessprogramming.wikispaces.com/Perft+Results
 **/
int checks = 0, captures = 0;
int checkmates = 0, ep = 0;
int promotions = 0;
unsigned long perft(ChessBoard* start, ChessMoveGenerator* gen, 
    int depth){
    move_t *next = NULL;
    int nextCount = 0;
    if(depth==0){
        if(ChessBoard_testForCheck(start)){
            //printf("\n");
            //ChessBoard_print(start);
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

int runPerftTest(const char* start, int perft3){
    ChessBoard* board = ChessBoard_new(start);
    ChessMoveGenerator* gen = ChessMoveGenerator_new(board);
    int ret = perft(board, gen, 3)!=perft3;
    ChessMoveGenerator_delete(gen);
    ChessBoard_delete(board);
    return ret;
}

int runPerftTests(){
    initZobrist();
    int i;
    int error = 0;
    for(i = 0; i < 6; i++){
        if(runPerftTest(PERFT_STARTS[i], PERFT3_VALUES[i])){
            error = 1;
            printf("Perft error for starting possition %d\n", i+1);
        }
    }
    closeZobrist();
    return error;
}

int runHeuristicsTests(){
    initZobrist();
    ChessBoard* board = ChessBoard_new(POS_2);
    initChessHeuristics(board);
    ChessMoveGenerator* gen = ChessMoveGenerator_new(board);

    ChessBoard_print(board);
    ChessHNode* root = ChessHNode_new(NULL, board);

    ChessHNode_expandBranches(root, gen);
    ChessBoard_makeMove(board, root->children[47]->move);
    printf("\n");
    ChessBoard_print(board);

    ChessHNode_expandLeaves(root->children[47], gen);
    int i;
    int minEval = 10000, minI;
    ChessHNode* child;
    for(i = 0; i < root->children[47]->childrenCount; i++){
        child = root->children[47]->children[i];
        if(child->evaluation < minEval){
            minEval = child->evaluation;
            minI = i;
        }
    }
    ChessBoard_makeMove(board, root->children[47]->children[minI]->move);
    printf("\n");
    ChessBoard_print(board);

    ChessHNode_delete(root);
    ChessMoveGenerator_delete(gen);
    ChessBoard_delete(board);

    closeChessHeuristics();
    closeZobrist();
    
    return 0;
}

int runSearchTests(){
    initZobrist();
    ChessBoard* board = ChessBoard_new(POS_2);
    initChessHeuristics(board);

    ChessBoard_print(board);
    move_t line[MAX_LINE_LENGTH];
    int length;
    int eval = getBestLine(board, 4, line, &length);
    printf("Eval: %d\n", eval);
    printf("Length: %d\n", length);
    int i;
    char moveOut[10];
    int moveOutSize;
    for(i = 0; i < length; i++){
        toAlgebraicNotation(line[i], board, moveOut, &moveOutSize);
        printf("%s\n", moveOut);
        ChessBoard_makeMove(board, line[i]);
        ChessBoard_print(board);
    }


    ChessBoard_delete(board);
    closeChessHeuristics();
    closeZobrist();
}

int main(void){
    runSearchTests();
    return 0;
}
