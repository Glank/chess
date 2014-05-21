#ifndef CHESS_PNG_H_INCLUDE
#define CHESS_PNG_H_INCLUDE
#include "board.h"
#include "moves.h"
#include "narrator.h"
#include <stdio.h>
typedef struct PGNRecord PGNRecord;
typedef enum 
    {OTHER_RESULT, WHITE_VICTORY, BLACK_VICTORY, DRAW} result_e;
typedef struct MoveIterator MoveIterator;

PGNRecord* PGNRecord_newFromBoard(
    ChessBoard* board, int drawAssumable);
PGNRecord* PGNRecord_newFromString(char* str);
PGNRecord* PGNRecord_newFromFile(FILE* fp, int* eofFlag);
void PGNRecord_delete(PGNRecord* self);
result_e PGNRecord_getResult(PGNRecord* self);
//needs to be deleted when finished
MoveIterator* PGNRecord_getMoveIterator(PGNRecord* self);
//the returned string is a new memory instance
char* PGNRecord_toString(PGNRecord* self);
ChessBoard* PGNRecord_toBoard(PGNRecord* self);
void PGNRecord_writeToFile(PGNRecord* self, FILE* fp);

void MoveIterator_delete(MoveIterator* self);
int MoveIterator_hasNext(MoveIterator* self);
move_t MoveIterator_getNext(MoveIterator* self);
#endif
