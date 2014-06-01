#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "bitboard.h"
#include "chesserrors.h"

#define BB_TOPLAY_MASK       1
#define BB_WK_CASTLE_MASK    2
#define BB_WQ_CASTLE_MASK    4
#define BB_BK_CASTLE_MASK    8
#define BB_BQ_CASTLE_MASK   16
#define BB_EN_PASSANT_MASK  224 
#define BB_EN_PASSANT_OFF   5

//private
struct BitBoard{
    int flags;
    uint64_t pieceSet[2][6]; //by side and type
};
void BitBoard_movePiece(BitBoard* self, int color, int type, location_t from, location_t to){
    self->pieceSet[color][type]^=((((uint64_t)1)<<from)|(((uint64_t)1)<<to));
}
void BitBoard_togglePiece(BitBoard* self, int color, int type, location_t loc){
    self->pieceSet[color][type]^=((uint64_t)1)<<loc;
}
void BitBoard_toggleToPlay(BitBoard* self){
    self->flags^=BB_TOPLAY_MASK;
}
void BitBoard_toggleWhiteKingSideCastle(BitBoard* self){
    self->flags^=BB_WK_CASTLE_MASK;
}
void BitBoard_toggleWhiteQueenSideCastle(BitBoard* self){
    self->flags^=BB_WQ_CASTLE_MASK;
}
void BitBoard_toggleBlackKingSideCastle(BitBoard* self){
    self->flags^=BB_BK_CASTLE_MASK;
}
void BitBoard_toggleBlackQueenSideCastle(BitBoard* self);
int BitBoard_canWhiteKingSideCastle(BitBoard* self);
int BitBoard_canWhiteQueenSideCastle(BitBoard* self);
int BitBoard_canBlackKingSideCastle(BitBoard* self);
int BitBoard_canBlackQueenSideCastle(BitBoard* self);
void BitBoard_setEnPassant(BitBoard* self, location_t loc);
void BitBoard_setHalfMoveClock(BitBoard* self, int value);
void BitBoard_setMoveNumber(BitBoard* self, int value);
int BitBoard_validate(BitBoard* self);

//public
BitBoard* BitBoard_new(){
    BitBoard* self = (BitBoard*)malloc(sizeof(BitBoard));
    int color; int type;
    for(color=0; color<2; color++){
        for(type=0; type<6; type++)
            self->pieceSet[color][type] = 0;
    }
    return self;
}
void BitBoard_delete(BitBoard* self){
    free(self);
}
int BitBoard_setUp(BitBoard* self, char* fen){
    char* tokens[13];
    int tokenLengths[13];

    {//Tolkenize the FEN
        int tokenStart, tokenEnd, tokenIndex;
        tokenStart=tokenEnd=tokenIndex = 0;
        tokens[0] = fen;
        int len = strlen(fen);
        while(tokenEnd<len){
            if(tokenIndex<7 && fen[tokenEnd]=='/'){
                tokenLengths[tokenIndex++]=tokenEnd-tokenStart;
                tokenStart=tokenEnd=tokenEnd+1;
                tokens[tokenIndex] = fen[tokenStart];
            }
            else if(tokenIndex>=7 && fen[tokenEnd]==' '){
                tokenLengths[tokenIndex++]=tokenEnd-tokenStart;
                tokenStart=tokenEnd=tokenEnd+1;
                if(tokenIndex>=13){
                    setError(PARSING_ERROR, "Too many tokens.");
                    return 1;
                }
                tokens[tokenIndex] = fen[tokenStart];
            }
            else if(tokenIndex>=7 && fen[tokenEnd]=='/'){
                setError(PARSING_ERROR, "Too many ranks.");
                return 1;
            }
            tokenEnd++;
        }
        tokenLengths[tokenIndex]=tokenEnd-tokenStart;
        if(tokenIndex<8){
            setError(PARSING_ERROR, "Too few ranks.");
            return 1;
        }
        if(tokenIndex<12){
            setError(PARSING_ERROR, "Too few tokens.");
            return 1;
        }
    }
    //setup bit board
    {//parse the first 8 tokens (which are each ranks in reverse order)
        int r, f, i; //rank, file, index in token
        int lastWasDigit;
        char lookup[] = "PpBbNnRrQqKk";
        for(r=7; r>=0; r--){
            lastWasDigit = 0;
            f = 0;
            for(i=0; i<tokenLengths[r]; i++){
                if(isDigit(tokens[r][i])){
                    if(lastWasDigit){
                        char err[1024];
                        sprintf(err, "Two or more sequentially digits in rank %d.", (r+1));
                        setError(PARSING_ERROR, err);
                        return 1;
                    }
                    lastWasDigit = 1;
                    f+=digitToInt(tokens[r][i]);
                }
                else{
                    //parse letters into characters
                    lastWasDigit = 0;
                    int n = getCharIndex(tokens[r][i]);
                    if(n==-1){
                        char err[1024];
                        sprintf(err, "Invalid character '%c' in rank %d.", tokens[r][i], (r+1));
                        setError(PARSING_ERROR, err);
                        return 1;
                    }
                    int color = n%2;
                    int type = n/2;
                    BitBoard_togglePiece(self, color, type, RANK_FILE(r,f));
                    f++;
                }
                if(f>=8){
                    char err[1024];
                    sprintf(err, "File overflow in rank %d.", (r+1));
                    setError(PARSING_ERROR, err);
                    return 1;
                }
            }
        }
    }//end rank parsing
    {//parse the last 5 tokens (to play, castling, en passant, half move clock, move number)
        int invalidToken=0;
        char invalidTokenName[32];

        //to-play
        if(tokenLengths[8]!=1){
            invalidToken = 8;
            invalidTokenName = "to-play flag";
            goto invalid;
        }
        char toPlayFlag = tokens[8][0];
        if(toPlayFlag!='w' && toPlayFlag!='b'){
            invalidToken = 8;
            invalidTokenName = "to-play flag";
            goto invalid;
        }
        if(toPlayFlag=='b')
            BitBoard_toggleToPlay(self);

        //castling
        if(tokenLengths[9]==1){
            if(tokens[9][0]!='-') goto invalid_castling;
        }
        else{
            int i;
            for(i = 0; i < tokenLengths[9]; i++){
                if(tokens[9][i]=='K'){
                    if(BitBoard_canWhiteKingSideCastle(self)) goto invalid_casting;
                    BitBoard_toggleWhiteKingSideCastle(self);
                }
                else if(tokens[9][i]=='Q'){
                    if(BitBoard_canWhiteQueenSideCastle(self)) goto invalid_casting;
                    BitBoard_toggleWhiteQueenSideCastle(self);
                }
                else if(tokens[9][i]=='k'){
                    if(BitBoard_canBlackKingSideCastle(self)) goto invalid_casting;
                    BitBoard_toggleBlackKingSideCastle(self);
                }
                else if(tokens[9][i]=='q'){
                    if(BitBoard_canBlackQueenSideCastle(self)) goto invalid_casting;
                    BitBoard_toggleBlackQueenSideCastle(self);
                }
                else goto invalid_castling;
            }
        }
        goto valid_casting;
        invalid_casting:
            invalidToken = 9;
            invalidTokenName = "castling flags";
            goto invalid;
        valid_castling:;

        //en passant
        if(tokenLengths[10]==1){
            if(tokens[10][0]!='-') goto invalid_enpassant;
        }
        else if(tokenLengths[10]==2){
            int rank, file;
            rank = tokens[10][0]-'a';
            file = tokens[10][1]-'1';
            if(rank < 0 || rank >= 8) goto invalid_enpassant;
            if(file < 0 || file >= 8) goto invalid_enpassant;
            BitBoard_setEnPassant(self, RANK_FILE(rank, file));
        }
        else goto invalid_enpassant;
        goto valid_enpassant;
        invalid_enpassant:
            invalidToken = 10;
            invalidTokenName = "en passant token";
            goto invalid;
        valid_enpassant:;
        
        {//half move clock
            int halfMoveClock;
            char nullTerminatedCopy[10];
            if(tokenLengths[11]>9) goto invalid_halfclock; 
            int n = tokenLengths[11]<9?tokenLengths[11]:9;
            strncpy(nullTerminatedCopy, tokens[11], n);
            nullTerminatedCopy[n] = '\0';
            int result = sscanf(nullTerminatedCopy, "%d", &halfMoveClock);
            if(result==0 || result==EOF) goto invalid_halfclock;
            BitBoard_setHalfMoveClock(self, halfMoveClock);
        }
        goto valid_halfclock;
        invalid_halfclock:
            invalidToken = 11;
            invalidTokenName = "half move clock";
            goto invalid;
        valid_halfclock:;

        {//move number
            int moveNumber;
            char nullTerminatedCopy[10];
            if(tokenLengths[12]>9) goto invalid_halfclock; 
            int n = tokenLengths[12]<9?tokenLengths[12]:9;
            strncpy(nullTerminatedCopy, tokens[12], n);
            nullTerminatedCopy[n] = '\0';
            int result = sscanf(nullTerminatedCopy, "%d", &moveNumber);
            if(result==0 || result==EOF) goto invalid_movenumber;
            BitBoard_setMoveNumber(self, moveNumber);
        }
        goto valid_movenumber;
        invalid_movenumber:
            invalidToken = 12;
            invalidTokenName = "move number";
            goto invalid;
        valid_movenumber:;
        
        //catch parsing errors
        goto valid;
        invalid:;
            char badToken[10];
            int n = tokenLengths[invalidToken]<9?tokenLengths[invalidToken]:9;
            strncpy(badToken, tokens[invalidToken], n);
            badToken[n] = '\0';
            char err[1024];
            sprintf(err, "Invalid %s: '%s'.", invalidTokenName, badToken);
            setError(PARSING_ERROR, err);
            return 1;
        valid:;
    }//end of last 5 tokens parsing
}
