#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <limits.h>
#include <assert.h>
#include <signal.h>
#include "board.h"
#include "zobrist.h"
#include "moves.h"
#include "heuristics.h"
#include "search.h"
#include "narrator.h"
#include "threads.h"
#include "pgn.h"
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
#define POS_7 "6k1/p2nq2p/8/2pb4/5p1P/1P3N2/PB2Q2K/R5b1 w - - 0 0"
#define POS_7_1 "6k1/p6p/8/2pbQ3/5p1q/1P3N2/PB4K1/R5b1 w - - 0 0"
#define POS_8 "5Bk1/p1Q2ppp/4p3/1b6/4PN2/P1P2P1K/6PP/4q3 b - - 0 0"
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
    int p = perft(board, gen, 3);
    int ret = p!=perft3;
    if(ret) printf("%d - ", p);
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
    ChessHEngine* engine = ChessHEngine_new(board);

    ChessBoard_print(board);
    ChessHNode* root = ChessHNode_new(NULL, engine);

    ChessHNode_expand(root, engine);
    ChessBoard_makeMove(board, root->children[47]->move);
    printf("\n");
    ChessBoard_print(board);

    ChessHNode_expand(root->children[47], engine);
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
    ChessHEngine_delete(engine);
    ChessBoard_delete(board);

    closeZobrist();
    
    return 0;
}

int runSearchTest(char* start, int depth){
    initZobrist();
    ChessBoard* board = ChessBoard_new(start);
    TTable* table = TTable_new();
    SearchThread* thread = SearchThread_new(board, table);
    ChessBoard_print(board);
    SearchThread_setTimeout(thread, 5);

    SearchThread_start(thread);
    SearchThread_join(thread);
    move_t line[MAX_LINE_LENGTH];
    int length;
    int eval = SearchThread_getBestLine(thread, line, &length);
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
        printf("%016llX\n", (long long unsigned int)board->hash);
    }

    SearchThread_delete(thread);
    TTable_delete(table);
    ChessBoard_delete(board);
    closeZobrist();
    return 0;
}

int runSearchTests(){
    runSearchTest(POS_8, 4);
    return 0;
}

int runGenTest(char* start){
    initZobrist();
    ChessBoard* board = ChessBoard_new(start);
    ChessMoveGenerator* gen = ChessMoveGenerator_new(board);
    ChessMoveGenerator_generateMoves(gen, ChessBoard_testForCheck(board), NULL);

    ChessBoard_print(board);
    printf("###############\n");
    char moveOut[10];
    int moveOutSize;
    int i;
    for(i = 0; i < gen->nextCount; i++){
        toAlgebraicNotation(gen->next[i], board, moveOut, &moveOutSize);
        printf("%s\n", moveOut);
        printf("%016llX\n", (long long unsigned int)board->hash);
        ChessBoard_makeMove(board, gen->next[i]);
        ChessBoard_print(board);
        printf("\n");
        ChessBoard_unmakeMove(board);
    }

    ChessBoard_delete(board);
    ChessMoveGenerator_delete(gen);
    closeZobrist();
    return 0;
}

int runAlgebraicNotationTest(){
    initZobrist();
    ChessBoard* board = ChessBoard_new(FEN_START);

    move_t move;
    move = fromAlgebraicNotation("e4", board);
    ChessBoard_makeMove(board, move);
    move = fromAlgebraicNotation("d5", board);
    ChessBoard_makeMove(board, move);
    ChessBoard_print(board);

    ChessBoard_delete(board);
    closeZobrist();
    return 0;
}

typedef struct TestThreadArgs TestThreadArgs;
struct TestThreadArgs{
    char* name;
    int* count;
    ChessMutex* mutex;
};

void* testThreadFunction(void* args){
    TestThreadArgs* threadArgs = (TestThreadArgs*)args;
    while(*(threadArgs->count) < 10){
        ChessMutex_lock(threadArgs->mutex);
        int oldCount = *(threadArgs->count);
        if(oldCount<10){
            ChessThread_sleep(100);
            *(threadArgs->count) = oldCount+1;
            printf("%s: %d\n", threadArgs->name, *(threadArgs->count));
        }
        ChessMutex_unlock(threadArgs->mutex);
    }
    return NULL;
}

int runThreadTests(){
    TestThreadArgs args1;
    TestThreadArgs args2;
    ChessMutex* mutex = ChessMutex_new();
    int count = 0;
    args1.name = "Thread 1";
    args1.mutex = mutex;
    args1.count = &count;
    args2.name = "Thread 2";
    args2.mutex = mutex;
    args2.count = &count;
    ChessThread* t1 = ChessThread_new(&testThreadFunction);
    ChessThread* t2 = ChessThread_new(&testThreadFunction);
    ChessThread_start(t1, &args1);
    ChessThread_start(t2, &args2);
    ChessThread_join(t1, NULL);
    ChessThread_join(t2, NULL);
    ChessMutex_delete(mutex);
    ChessThread_delete(t1);
    ChessThread_delete(t2);
    return 0;
}

volatile sig_atomic_t flag = 0;
void sigTestFunction(int sig){
    flag = 1;
}
int runSigTest(){
    signal(SIGINT, sigTestFunction); 
    while(!flag)
        ChessThread_sleep(50);
    printf("Exiting.\n");
    ChessThread_sleep(5000);
    return 0;
}

int runPGNTest(){
    initZobrist();
    ChessBoard* board = ChessBoard_new(FEN_START);
    ChessBoard_makeMove(board, fromAlgebraicNotation("f3", board));
    ChessBoard_makeMove(board, fromAlgebraicNotation("e5", board));
    ChessBoard_makeMove(board, fromAlgebraicNotation("g4", board));
    ChessBoard_makeMove(board, fromAlgebraicNotation("Qh4++", board));
    ChessBoard_print(board);

    PGNRecord* pgn = PGNRecord_newFromBoard(board, 1);
    char* out = PGNRecord_toString(pgn);
    printf("%s\n", out);
    free(out);

    PGNRecord_delete(pgn);
    ChessBoard_delete(board);
    closeZobrist();
    return 0;
}

int main(void){
    //runPerftTests();
    //runSearchTests();
    //runGenTest(POS_4);
    //runAlgebraicNotationTest();
    //runThreadTests();
    //runSigTest();
    runPGNTest();
    return 0;
}
