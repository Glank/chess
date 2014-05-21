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
#include "opening.h"
#include "mind.h"
#include "game.h"

int puzzle_main(int argc, char** argv){
    printf("Press Ctl-C to exit.\n");
    initZobrist();
    ChessBoard* board = ChessBoard_new(argv[1]);
    ChessBoard_print(board);
    ChessMind* mind = ChessMind_new(board);
    ChessMind_puzzleSearch(mind);
    ChessMind_delete(mind);
    ChessBoard_delete(board);
    closeZobrist();
    return 0;
}

int game_main(int argc, char** argv){
    ChessGame* game = ChessGame_new();
    int argIndex = 1;
    int player;
    int seconds;
    for(player=0; player<2; player++){
        if(strcmp(argv[argIndex], "h")==0){
            ChessGame_setHuman(game, player, 1);
            argIndex++;
        }
        else{
            assert(strcmp(argv[argIndex], "c")==0);
            ChessGame_setHuman(game, player, 0);
            argIndex++;
            if(argIndex<argc && strcmp(argv[argIndex],"-t")==0){
                argIndex++;
                sscanf(argv[argIndex], "%d", &seconds);
                ChessGame_setTimeout(game, player, seconds);
                argIndex++;
            }
        }
    }
    if(argIndex<argc){
        if(strcmp(argv[argIndex], "-fen")==0){
            argIndex++;
            printf("FEN: '%s'\n", argv[argIndex]);
            ChessGame_setFEN(game, argv[argIndex++]);
        }
        else if(strcmp(argv[argIndex], "-pgn")==0){
            argIndex++;
            printf("PGN File: '%s'\n", argv[argIndex]);
            ChessGame_setPGNFile(game, argv[argIndex++]);
        }
    }

    initZobrist();
    ChessGame_play(game);
    ChessGame_delete(game);
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
    printf("  You must input moves in PGN algebraic notation - capitolization counts.\n\n");
    printf("  The -t parameter may be included for either computer player if you want to specify the number of \n");
    printf("  seconds the AI will think (by default 10)\n");
    printf("    ./chess -g h c -t 60\n");
    printf("  You can also play a game from any FEN starting possition:\n");
    printf("    ./chess -g c h \"rnbqkbnr/pp1ppppp/8/2p5/4P3/5N2/PPPP1PPP/RNBQKB1R b KQkq - 1 2\"\n\n");
    printf("  The opening interface, -o, may be used to:\n");
    printf("    -c: compile the opening book (can take upwards of an hour),\n");
    printf("    -p: print the opening book for\n");
    printf("    -m: minimum number occurences in the source game database,\n");
    printf("    -r: or generate a random opening..\n");
    printf("    ./chess -o -p -m 25 -r\n\n");
}

int opening_main(int argc, char** argv){
    int depth = 25;
    int min = 25; //just for printing
    initZobrist();
    int i;
    int compile=0, print=0, genRandom=0;
    for(i = 1; i<argc; i++){
        if(strcmp(argv[i], "-c")==0)
            compile = 1;
        else if(strcmp(argv[i], "-p")==0)
            print = 1;
        else if(strcmp(argv[i], "-r")==0)
            genRandom = 1;
        else if(strcmp(argv[i], "-m")==0){
            i++;
            if(i==argc){
                printf("Invalid use of '-m' arg.\n");
                return 1;
            }
            sscanf(argv[i], "%d", &min);
        }
        else{
            printf("Unknown argument: '%s'\n", argv[i]);
            return 1;
        }
    }
    if(compile)
        OpeningBook_generate(OPENING_SOURCE, OPENING_BINARY, depth);
    if(print || genRandom){
        OpeningBook* book = OpeningBook_load(OPENING_BINARY);
        if(print)
            OpeningBook_print(book, min);
        if(genRandom){
            ChessBoard* board = ChessBoard_new(FEN_START);
            move_t move;
            while(OpeningBook_hasLine(book, board, min)){
                move = OpeningBook_randNextMove(book, board, min);
                ChessBoard_makeMove(board, move);
            }
            PGNRecord* pgn = PGNRecord_newFromBoard(board, 1);
            char* out = PGNRecord_toString(pgn);
            printf("%s\n", out);
            free(out);
            ChessBoard_print(board);
            ChessBoard_delete(board);
        }
        OpeningBook_delete(book);
    }
    closeZobrist();
    return 0;
}

int main(int argc, char** argv){
    if(argc==3 && strcmp(argv[1], "-p")==0)
        return puzzle_main(argc-1, argv+1);
    else if(argc>3 && strcmp(argv[1], "-g")==0)
        return game_main(argc-1, argv+1);
    else if(argc>=2 && strcmp(argv[1], "-o")==0)
        return opening_main(argc-1, argv+1);
    else
        printUsage();

    return 0;
}
