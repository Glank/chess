#include "mind.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>
struct ChessMind{
    OpeningBook* book;
    TTable* table;
    ChessBoard* board;
    ChessBoard* ponderingBoard;
    SearchThread* pondering;
    int minOpeningGames;
    int timeout;
};

ChessMind* ChessMind_new(ChessBoard* board){
    ChessMind* self = (ChessMind*)malloc(sizeof(ChessMind));
    if(board->standard)
        self->book = OpeningBook_load(OPENING_BINARY);
    else
        self->book = NULL;
    self->table = TTable_new();
    self->minOpeningGames = 4;
    self->timeout = 10;
    self->board = board;
    return self;
}
void ChessMind_delete(ChessMind* self){
    OpeningBook_delete(self->book);
    TTable_delete(self->table);
    free(self);
}
void ChessMind_setTimeout(ChessMind* self, int timeout){
    self->timeout = timeout;
}
void ChessMind_puzzleSearch(ChessMind* self){
    SearchThread* thread = SearchThread_new(self->board, self->table);
    SearchThread_setTimeout(thread, 0);
    SearchThread_setPrintEachNewLine(thread, 1);
    SearchThread_start(thread);
    SearchThread_join(thread);
    //probably won't be reached, but just in case
    SearchThread_delete(thread);
}
void ChessMind_makeMove(ChessMind* self){
    move_t move;
    if(self->book!=NULL && 
        OpeningBook_hasLine(self->book, self->board, 
        self->minOpeningGames)){
        move = OpeningBook_randNextMove(self->book, self->board,
            self->minOpeningGames);
    }
    else{
        SearchThread* thread = SearchThread_new(self->board,
            self->table);
        SearchThread_setTimeout(thread, self->timeout);
        SearchThread_start(thread);
        SearchThread_join(thread);
        move = SearchThread_getBestMove(thread);
        SearchThread_delete(thread);
    }
    char moveOut[10];
    int moveOutLength;
    toAlgebraicNotation(move, self->board, moveOut, &moveOutLength);
    printf("%s\n", moveOut);
    ChessBoard_makeMove(self->board, move);
}
void ChessMind_startPondering(ChessMind* self){
    self->ponderingBoard = ChessBoard_copy(self->board);
    self->pondering = SearchThread_new(self->ponderingBoard,
        self->table);
    SearchThread_setTimeout(self->pondering, 0); //don't timeout
    SearchThread_start(self->pondering);
}
void ChessMind_stopPondering(ChessMind* self){
    SearchThread_stop(self->pondering);
    ChessBoard_delete(self->ponderingBoard);
}

struct ChessGame{
    int differentFEN;
    char fen[64];
    int isHuman[2];
    int timeout[2];
    ChessBoard* board;
    ChessMind* minds[2];
    int toPlay;
};
ChessGame* ChessGame_new(){
    ChessGame* self = (ChessGame*)malloc(sizeof(ChessGame));
    self->differentFEN = 0;
    int player;
    for(player=0; player<2; player++){
        self->isHuman[player] = 1;
        self->timeout[player] = 10;
        self->minds[player] = NULL;
    }
    self->board = NULL;
    self->toPlay = 0;
    return self;
}
void ChessGame_delete(ChessGame* self){
    int player;
    for(player=0; player<2; player++){
        if(self->minds[player]!=NULL)
            ChessMind_delete(self->minds[player]);
    }
    if(self->board!=NULL)
        ChessBoard_delete(self->board);
    free(self);
}
void ChessGame_setFEN(ChessGame* self, char* fen){
    strcpy(self->fen, fen);
    self->differentFEN = 1;
}
void ChessGame_setHuman(ChessGame* self, int player, int human){
    self->isHuman[player] = human;
}
void ChessGame_setTimeout(ChessGame* self, int player, int timeout){
    self->timeout[player] = timeout;
}
int ChessGame_gameOver(ChessGame* self){
    if(ChessBoard_testForStalemate(self->board) ||
        ChessBoard_isInOptionalDraw(self->board)){
        printf("1/2-1/2\n");
        return 1;
    }
    if(ChessBoard_testForCheckmate(self->board)){
        if(self->toPlay)
            printf("1-0\n");
        else
            printf("0-1\n");
        return 1;
    }
    return 0;
}
void ChessGame_doHumanMove(ChessGame* self){
    int otherPlayer = self->toPlay?0:1;
    if(!self->isHuman[otherPlayer])
        ChessMind_startPondering(self->minds[otherPlayer]);
    char* line = getLine();
    move_t move = fromAlgebraicNotation(line, self->board);
    while(move==0){
        printf("Invalid Move.\n");
        line = getLine();
        move = fromAlgebraicNotation(line, self->board);
    }
    if(!self->isHuman[otherPlayer])
        ChessMind_stopPondering(self->minds[otherPlayer]);
    ChessBoard_makeMove(self->board, move);
}
void ChessGame_makeNextMove(ChessGame* self){
    if(self->isHuman[self->toPlay])
        ChessGame_doHumanMove(self);
    else
        ChessMind_makeMove(self->minds[self->toPlay]);
    self->toPlay = self->toPlay?0:1;
}
void ChessGame_play(ChessGame* self){
    if(self->differentFEN)
        self->board = ChessBoard_new(self->fen);
    else
        self->board = ChessBoard_new(FEN_START);
    assert(self->board!=NULL);
    int player;
    for(player=0;player<2;player++){
        if(!self->isHuman[player]){
            self->minds[player] = ChessMind_new(self->board);
            ChessMind_setTimeout(self->minds[player],
                self->timeout[player]);
        }
    }
    self->toPlay = self->board->flags&TO_PLAY_FLAG?1:0;
    ChessBoard_print(self->board);
    while(!ChessGame_gameOver(self)){
        ChessGame_makeNextMove(self);
        ChessBoard_print(self->board);
    }
}
