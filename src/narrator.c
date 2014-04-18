#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "narrator.h"


char __pieceTypeToChar(pieceType_e type){
    switch(type){
    case KING:
        return 'K';
    case QUEEN:
        return 'Q';
    case ROOK:
        return 'R';
    case BISHOP:
        return 'B';
    case KNIGHT:
        return 'N';
    default:
        assert(0);
    }
}

char __fileToChar(int file){
    return 'a'+file;
}

char __rankToChar(int rank){
    return '1'+rank;
}

void toAlgebraicNotation(move_t move, ChessBoard* board, char* out, int* outSize){
    location_t from = GET_FROM(move);
    location_t to = GET_TO(move);
    int meta = GET_META(move);
    int i = 0;
    ChessBoard_makeMove(board, move);
    int isInCheck = ChessBoard_testForCheck(board);
    ChessBoard_unmakeMove(board);

    if(meta==KING_CASTLE_MOVE){
        char* notation = "O-O\0";
        for(i = 0; i < 4; i++)
            out[i] = notation[i];
        (*outSize)=3;
        return;
    }
    if(meta==QUEEN_CASTLE_MOVE){
        char* notation = "O-O-O\0";
        for(i = 0; i < 6; i++)
            out[i] = notation[i];
        (*outSize)=5;
        return;
    }

    //write piece indicator
    ChessPiece* piece = board->squares[from];
    if(piece->type==KING)
        out[i++] = 'K';
    else{
        ChessMoveGenerator* gen = ChessMoveGenerator_new(board);
        ChessMoveGenerator_generateMoves(gen, isInCheck, NULL);
        //for each other move, check to see if there are multiple pieces
        //of this piece's type that can move to the 'to' square.
        int j;
        move_t otherMoves[10];
        int otherMovesCount = 0;
        move_t otherMove;
        for(j = 0; j < gen->nextCount; j++){
            otherMove = gen->next[j];
            if((GET_TO(otherMove)==to) &&
                (board->squares[GET_FROM(otherMove)]->type==piece->type)){
                if(piece->type==PAWN && ((move&CAPTURE_MOVE_FLAG)!=(otherMove&CAPTURE_MOVE_FLAG)))
                    continue;
                if(otherMove==move)
                    continue;
                otherMoves[otherMovesCount++] = otherMove;
            }
        }
        int sameRank = 0, sameFile = 0;
        for(j = 0; j < otherMovesCount; j++){
            otherMove = otherMoves[j];
            if(GET_RANK(move)==GET_RANK(GET_FROM(otherMove)))
                sameRank = 1;
            if(GET_FILE(move)==GET_FILE(GET_FROM(otherMove)))
                sameFile = 1;
        }
        
        if(piece->type!=PAWN)
            out[i++] = __pieceTypeToChar(piece->type);
        if(otherMovesCount){
            if(!sameFile)
                out[i++] = __fileToChar(GET_FILE(from));
            else if(!sameRank)
                out[i++] = __rankToChar(GET_RANK(from));
            else{
                out[i++] = __fileToChar(GET_FILE(from));
                out[i++] = __rankToChar(GET_RANK(from));
            }
        }
        ChessMoveGenerator_delete(gen);
    }
    //if capture
    if(meta&CAPTURE_MOVE_FLAG)
        out[i++] = 'x';
    //write destination
    out[i++] = __fileToChar(GET_FILE(to));
    out[i++] = __rankToChar(GET_RANK(to));
    if(isInCheck)
        out[i++] = '+';
    out[i] = '\0';
    (*outSize) = i;
}