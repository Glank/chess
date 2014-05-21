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
    if(self->book!=NULL)
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
