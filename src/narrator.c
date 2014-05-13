#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
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
    ChessMoveGenerator* gen = ChessMoveGenerator_new(board);
    ChessMoveGenerator_generateMoves(gen, isInCheck, NULL);
    int isInCheckmate = isInCheck&&(gen->nextCount==0);
    ChessBoard_unmakeMove(board);
    ChessMoveGenerator_generateMoves(gen, ChessBoard_testForCheck(board), NULL);

    if(meta==KING_CASTLE_MOVE){
        char* notation = "O-O";
        for(i = 0; i < 3; i++)
            out[i] = notation[i];
    }
    else if(meta==QUEEN_CASTLE_MOVE){
        char* notation = "O-O-O";
        for(i = 0; i < 5; i++)
            out[i] = notation[i];
    }
    else{
        //write piece indicator
        ChessPiece* piece = board->squares[from];
        if(piece->type==KING)
            out[i++] = 'K';
        else{
            //for each other move, check to see if there are multiple pieces
            //of this piece's type that can move to the 'to' square.
            int j;
            move_t otherMoves[20];
            int otherMovesCount = 0;
            move_t otherMove;
            for(j = 0; j < gen->nextCount; j++){
                otherMove = gen->next[j];
                if((GET_TO(otherMove)==to) && (GET_FROM(otherMove)!=from) &&
                    (board->squares[GET_FROM(otherMove)]->type==piece->type)){
                    if(piece->type==PAWN && (move&CAPTURE_MOVE_FLAG)!=(otherMove&CAPTURE_MOVE_FLAG))
                        continue;
                    if(otherMove==move)
                        continue;
                    otherMoves[otherMovesCount++] = otherMove;
                }
            }
            //note if there are multiple moves from the same rank or file
            int sameRank = 0, sameFile = 0;
            for(j = 0; j < otherMovesCount; j++){
                otherMove = otherMoves[j];
                if(GET_RANK(from)==GET_RANK(GET_FROM(otherMove)))
                    sameRank = 1;
                if(GET_FILE(from)==GET_FILE(GET_FROM(otherMove)))
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
        }
        //if capture
        if(meta&CAPTURE_MOVE_FLAG)
            out[i++] = 'x';
        //write destination
        out[i++] = __fileToChar(GET_FILE(to));
        out[i++] = __rankToChar(GET_RANK(to));
        if(meta&PROMOTION_MOVE_FLAG){
            out[i++] = '=';
            switch(meta&PROMOTION_MASK){
                case KNIGHT_PROMOTION:
                    out[i++] = 'N';
                    break;
                case BISHOP_PROMOTION:
                    out[i++] = 'B';
                    break;
                case ROOK_PROMOTION:
                    out[i++] = 'R';
                    break;
                case QUEEN_PROMOTION:
                    out[i++] = 'Q';
                    break;
                default:
                    assert(0);
            }
        }
    }
    if(isInCheckmate)
        out[i++] = '#';
    else if(isInCheck)
        out[i++] = '+';
    out[i] = '\0';
    (*outSize) = i;

    ChessMoveGenerator_delete(gen);
}

move_t fromAlgebraicNotation(char* notation, ChessBoard* board){
    ChessMoveGenerator* gen = ChessMoveGenerator_new(board);
    ChessMoveGenerator_generateMoves(gen, ChessBoard_testForCheck(board), NULL);
    int i;
    char match[10];
    int matchLength;
    move_t move;
    for(i = 0; i < gen->nextCount; i++){
        toAlgebraicNotation(gen->next[i], board, match, &matchLength);
        if(strcmp(notation, match)==0){
            move = gen->next[i];
            break;
        }
    }
    if(i==gen->nextCount)
        return 0;
    ChessMoveGenerator_delete(gen);
    return move;
}
