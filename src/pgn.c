#include "pgn.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>

//PRIVATE
typedef struct PGNNode PGNNode;
typedef struct PGNToken PGNToken;

struct PGNRecord{
    char* source;
    result_e result;
    PGNNode* movesHead;
    PGNNode* movesTail;
};

struct PGNNode{
    move_t move;
    PGNNode* next;
};
PGNNode* PGNNode_new(move_t move, PGNNode* next){
    PGNNode* self = (PGNNode*)
        malloc(sizeof(PGNNode));
    self->move = move;
    self->next = next;
    return self;
}
void PGNNode_delete(PGNNode* self){
    if(self->next!=NULL)
        PGNNode_delete(self->next);
    free(self);
}

typedef enum {
    HEAD_T, TAG_T, COMMENT_T, MOVE_NUMBER_T, MOVE_T, RESULT_T
} tokenType_e;
struct PGNToken{
    char* substring;
    int length;
    tokenType_e type;
    PGNToken* prev;
};
PGNToken* PGNToken_new(char* substring, int length, tokenType_e type,
    PGNToken* prev){
    PGNToken* self = (PGNToken*) malloc(sizeof(PGNToken));
    self->substring = substring;
    self->length = length;
    self->type = type;
    self->prev = prev;
    return self;
}
void PGNToken_delete(PGNToken* self){
    if(self->prev!=NULL)
        PGNToken_delete(self->prev);
    free(self);
}
PGNToken* PGNToken_readTagToken(char* source, PGNToken* tail){
    if(tail==NULL) //eof error
        return NULL;
    assert(source[0]=='[');
    int len;
    for(len=1;;len++){
        if(source[len-1]=='\0') //eof error
            return NULL;
        if(source[len-1]=='%')
            len++;
        else if(source[len-1]==']')
            break;
    }
    tail = PGNToken_new(source, len, TAG_T, tail);
    return tail;
}
PGNToken* PGNToken_readCommentToken(char* source, PGNToken* tail){
    if(tail==NULL) //eof error
        return NULL;
    assert(source[0]=='{');
    int len;
    for(len=1;;len++){
        if(source[len-1]=='\0') //eof error
            return NULL;
        if(source[len-1]=='%')
            len++;
        else if(source[len-1]=='}')
            break;
    }
    tail = PGNToken_new(source, len, COMMENT_T, tail);
    return tail;
}
PGNToken* PGNToken_readOtherToken(char* source, PGNToken* tail){
    if(tail==NULL) //eof error
        return NULL;
    int len;
    for(len=1;;len++){
        if(source[len]=='\0')
            break;
        if(isWhiteSpace(source[len]))
            break;
        if(strchr("[{",source[len])!=NULL)
            break;
        if(source[len-1]=='.')
            break;
    }
    tokenType_e type;
    if(source[len-1]=='.')
        type = MOVE_NUMBER_T;
    else if(len==3 && strncmp("1-0", source, 3))
        type = RESULT_T;
    else if(len==3 && strncmp("0-1", source, 3))
        type = RESULT_T;
    else if(len==7 && strncmp("1/2-1/2", source, 7))
        type = RESULT_T;
    else if(len==1 && strncmp("*", source, 1))
        type = RESULT_T;
    else
        type = MOVE_T;
    tail = PGNToken_new(source, len, type, tail);
    return tail;
}
PGNToken* PGNToken_tokenize(char* source){
    PGNToken* tail = PGNToken_new(NULL, 0, HEAD_T, NULL);
    while(*source!='\0'){
        if(isWhiteSpace(*source))
            source++;
        else if(*source=='['){
            tail = PGNToken_readTagToken(source, tail);
            if(tail==NULL)
                return NULL;
            source+=tail->length;
        }
        else if(*source=='{'){
            tail = PGNToken_readCommentToken(source, tail);
            if(tail==NULL)
                return NULL;
            source+=tail->length;
        }
        else{
            tail = PGNToken_readOtherToken(source, tail);
            if(tail==NULL)
                return NULL;
            source+=tail->length;
        }
    }
    return tail;
}

struct MoveIterator{
    PGNNode* cur;
};
MoveIterator* MoveIterator_new(PGNNode* cur){
    MoveIterator* self = (MoveIterator*)
        malloc(sizeof(MoveIterator));
    self->cur = cur;
    return self;
}

//PUBLIC
PGNRecord* PGNRecord_newFromBoard(ChessBoard* board, 
    int drawAssumable){
    PGNRecord* self = (PGNRecord*)
        malloc(sizeof(PGNRecord));
    self->source = NULL;
    //derive the game result
    ChessMoveGenerator* gen = ChessMoveGenerator_new(board);
    int inCheck = ChessBoard_testForCheck(board);
    ChessMoveGenerator_generateMoves(gen, inCheck, NULL);
    if(gen->nextCount==0){
        if(inCheck){
            if(gen->toPlay==WHITE)
                self->result=BLACK_VICTORY;
            else
                self->result=WHITE_VICTORY;
        }
        else
            self->result=DRAW;
    }
    else if(drawAssumable && ChessBoard_isInOptionalDraw(board))
        self->result=DRAW;
    else
        self->result=OTHER_RESULT;
    ChessMoveGenerator_delete(gen);
    //create the move list
    GameInfo* info = (GameInfo*)board->extra;
    if(info->movesCount==0){
        self->movesHead = NULL;
        self->movesTail = NULL;
    }
    else{
        //build the move list tail first
        PGNNode* cur = 
            PGNNode_new(info->moves[info->movesCount-1], NULL);    
        self->movesTail = cur;
        int i;
        for(i = info->movesCount-2; i>=0; i--)
            cur = PGNNode_new(info->moves[i], cur);
        self->movesHead = cur;
    }
    return self;
}
PGNRecord* PGNRecord_newFromString(char* str){
    PGNToken* tokenTail = PGNToken_tokenize(str);
    if(tokenTail==NULL)
        return NULL; //eof
    if(tokenTail->type != RESULT_T)
        return NULL; //did not end with result
    PGNRecord* self = (PGNRecord*)
        malloc(sizeof(PGNRecord));
    //read the result token
    //TODO
}
void PGNRecord_delete(PGNRecord* self){
    if(self->source!=NULL)
        free(self->source);
    if(self->movesHead!=NULL)
        PGNNode_delete(self->movesHead);
    free(self);
}
MoveIterator* PGNRecord_getMoveIterator(PGNRecord* self){
    return MoveIterator_new(self->movesHead);
}
char* PGNRecord_toString(PGNRecord* self){
    if(self->source!=NULL){
        char* srcCopy = (char*)
            malloc(strlen(self->source)+1);
        strcpy(srcCopy, self->source);
        return srcCopy;
    }
    char buffer[32] = "";
    int bufferLen = 0;
    int len = 0;
    int maxLen = 31;
    char* out = (char*)malloc(maxLen+1);
    out[0] = '\0';
    MoveIterator* iterator = PGNRecord_getMoveIterator(self);
    int halfMoveNumber = 0;
    move_t move;
    ChessBoard* board = ChessBoard_new(FEN_START);
    while(MoveIterator_hasNext(iterator)){
        move = MoveIterator_getNext(iterator);
        if(halfMoveNumber%2==0){
            if(halfMoveNumber)
                sprintf(buffer, " %d.", halfMoveNumber/2+1);
            else
                sprintf(buffer, "1.");
            out = str_unload_buffer(buffer, out, &len, &maxLen);
        }
        buffer[0] = ' ';
        toAlgebraicNotation(move, board, buffer+1, &bufferLen);
        ChessBoard_makeMove(board, move);
        out = str_unload_buffer(buffer, out, &len, &maxLen);
        halfMoveNumber++;
    }
    switch(self->result){
    case OTHER_RESULT:
        sprintf(buffer, " *");
        break;
    case WHITE_VICTORY:
        sprintf(buffer, " 1-0");
        break;
    case BLACK_VICTORY:
        sprintf(buffer, " 0-1");
        break;
    case DRAW:
        sprintf(buffer, " 1/2-1/2");
        break;
    default:
        assert(0);
    }
    out = str_unload_buffer(buffer, out, &len, &maxLen);
    ChessBoard_delete(board);
    MoveIterator_delete(iterator);
    return out;
}

void MoveIterator_delete(MoveIterator* self){
    free(self);
}
int MoveIterator_hasNext(MoveIterator* self){
    return self->cur!=NULL;
}
move_t MoveIterator_getNext(MoveIterator* self){
    move_t ret = self->cur->move;
    self->cur = self->cur->next;
    return ret;
}
