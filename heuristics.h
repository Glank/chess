#ifndef CHESS_HEURISTICS_H_INCLUDE
#define CHESS_HEURISTICS_H_INCLUDE
#include "board.h"

typedef enum {ESTIMATE, ABSOLUTE} evalType_e;
typedef enum {UN_EVAL, PRE_EVAL, FULL_EVAL} evalState_e;

typedef struct ChessHNode ChessHNode;
struct ChessHNode{
    move_t from;
    struct ChessHNode* parent;
    struct ChessHNode** children;
    int childrenCount;
    color_t toPlay;
    int inCheck;
    zob_hash_t hash;
    int halfMoveNumber;
    int depth;

    int evaluation;
    evalState_e state;
    evalType_e type;
}
ChessHNode* ChessHNode_new(ChessHNode* parent);
void ChessHNode_delete(ChessHNode* self);
void ChessHNode_doPreEvaluation(ChessHNode* self, ChessBoard* board);
#endif
