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

int puzzle_main(int argc, char** argv){
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
    SearchThread* thread = SearchThread_new(board);
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
    SearchThread_delete(thread);
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

void doMove(ChessBoard* board, int human){
    if(human)
        doHumanMove(board);
    else
        doAIMove(board, 60);
}

int ChessBoard_testForCheckmate(ChessBoard* self){
    if(!ChessBoard_testForCheck(self))
        return 0;
    ChessMoveGenerator* gen = ChessMoveGenerator_new(self);
    ChessMoveGenerator_generateMoves(gen, 1, NULL);
    int isInCheckmate = gen->nextCount==0;
    ChessMoveGenerator_delete(gen);
    return isInCheckmate;
}

int gameOver(ChessBoard* board){
    if(ChessBoard_isInOptionalDraw(board)){
        printf("1/2-1/2\n");
        return 1;
    }
    if(ChessBoard_testForCheckmate(board)){
        if(board->flags&TO_PLAY_FLAG)
            printf("0-1\n");
        else
            printf("1-0\n");
        return 1;
    }
    return 0;
}

int game_main(int argc, char** argv){
    int players[2];
    if(strcmp(argv[1], "h")==0)
        players[0] = 1;
    else{
        players[0] = 0;
        assert(strcmp(argv[1], "c")==0);
    }
    if(strcmp(argv[2], "h")==0)
        players[1] = 1;
    else{
        players[1] = 0;
        assert(strcmp(argv[2], "c")==0);
    }
    initZobrist();
    ChessBoard* board = ChessBoard_new(FEN_START);
    initChessHeuristics(board);
    ChessBoard_print(board);

    int player = 0;
    while(!gameOver(board)){
        doMove(board, players[player]);
        player = player?1:0;
    }
    
    ChessBoard_delete(board);
    closeChessHeuristics();
    closeZobrist();
    return 0;
}

int main(int argc, char** argv){
    if(strcmp(argv[1], "-p")==0)
        return puzzle_main(argc-1, argv+1);
    else if(strcmp(argv[1], "-g")==0)
        return game_main(argc-1, argv+1);

    printf("Invalid\n");
    return 1;
}
