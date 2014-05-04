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
    printf("Press Ctl-C to exit.\n");
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
}

void doMove(ChessBoard* board, int human, int seconds){
    if(human)
        doHumanMove(board);
    else
        doAIMove(board, seconds);
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
    int sec_specified = 0;
    int seconds = 10;
    if(argc>3 && strcmp(argv[3], "-s")==0){
        sec_specified = 1;
        sscanf(argv[4], "%d", &seconds);
    }
    initZobrist();
    ChessBoard* board;
    if(!sec_specified && argc>3)
        board = ChessBoard_new(argv[3]);
    else if(sec_specified && argc>5)
        board = ChessBoard_new(argv[5]);
    else
        board = ChessBoard_new(FEN_START);
    initChessHeuristics(board);

    int player = 0;
    while(!gameOver(board)){
        printf("%x\n", board->hash);
        ChessBoard_print(board);
        doMove(board, players[player], seconds);
        player = player?0:1;
    }
    printf("%x\n", board->hash);
    ChessBoard_print(board);
    
    ChessBoard_delete(board);
    closeChessHeuristics();
    closeZobrist();
    return 0;
}

void printUsage(){
    printf("\nUsage:\n\n");
    printf("  Puzzles may be input in FEN notation:\n");
    printf("    ./chess -p \"8/8/8/8/8/6K1/5R2/7k w - - 0 0\"\n\n");
    printf("  You may play against the computer:\n");
    printf("    ./chess -g h c\n");
    printf("  The 'h' and 'c' mean human player and computer player for the first\n  and second players respectively.\n");
    printf("  You must input moves in PNG algebraic notation - capitolization counts.\n\n");
    printf("  The -s parameter may be included if you want to specify the number of seconds the AI will think (by default 10)\n");
    printf("    ./chess -g h c -s 60\n");
    printf("  You can also play a game from any FEN starting possition:\n");
    printf("    ./chess -g c h \"rnbqkbnr/pp1ppppp/8/2p5/4P3/5N2/PPPP1PPP/RNBQKB1R b KQkq - 1 2\"\n\n");
}

int main(int argc, char** argv){
    if(argc==3 && strcmp(argv[1], "-p")==0)
        return puzzle_main(argc-1, argv+1);
    else if(argc>3 && strcmp(argv[1], "-g")==0)
        return game_main(argc-1, argv+1);
    else
        printUsage();

    return 0;
}
