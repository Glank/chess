#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <assert.h>
#include <string.h>
#include <signal.h>
#include "board.h"
#include "zobrist.h"
#include "moves.h"
#include "heuristics.h"
#include "search.h"
#include "narrator.h"
#include "strutl.h"

SearchThread* thread;
void interruptHook(int sig){
    SearchThread_stop(thread);
}

int main(int argc, char** argv){
    initZobrist();
    ChessBoard* board = ChessBoard_new(argv[1]);
    initChessHeuristics(board);
    thread = SearchThread_new(board);
    SearchThread_setTimeout(thread, 0);
    SearchThread_setPrintEachNewLine(thread, 1);

    ChessBoard_print(board);

    SearchThread_start(thread);
    SearchThread_join(thread);
    
    SearchThread_delete(thread);
    ChessBoard_delete(board);
    closeChessHeuristics();
    closeZobrist();
    return 0;
}

void doAIMove(ChessBoard* board, int seconds){
    thread = SearchThread_new(board);
    SearchThread_setTimeout(thread, seconds);
    SearchThread_start(thread);
    SearchThread_join(thread);
    move_t bestMove = SearchThread_getBestMove(thread);
    char moveOut[10];
    int moveOutLength;
    toAlgebraicNotation(bestMove, board, moveOut, &moveOutLength);
    printf("%s\n", moveOut);
    ChessBoard_makeMove(board, bestMove);
    ChessBoard_print(board);
}

void doHumanMove(ChessBoard* board){
    char* line = getLine();
    move_t move = fromAlgebraicNotation(line, board);
    while(move==0){
        printf("Invalid Move.\n");
        line = getLine();
        move = fromAlgebraicNotation(line, board);
    }
    ChessBoard_makeMove(board, move);
    ChessBoard_print(board);
}
