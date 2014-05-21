#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "game.h"
struct ChessGame{
    char* fen;
    int isHuman[2];
    int timeout[2];
    ChessBoard* board;
    ChessMind* minds[2];
    int toPlay;
};
ChessGame* ChessGame_new(){
    ChessGame* self = (ChessGame*)malloc(sizeof(ChessGame));
    self->fen = NULL;
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
    if(self->fen!=NULL)
        free(self->fen);
    free(self);
}
void ChessGame_setFEN(ChessGame* self, char* fen){
    self->fen = (char*)malloc(sizeof(char)*(strlen(fen)+1));
    strcpy(self->fen, fen);
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
    if(self->fen != NULL)
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
