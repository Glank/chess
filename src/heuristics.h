#ifndef CHESS_HEURISTICS_H_INCLUDE
#define CHESS_HEURISTICS_H_INCLUDE
#include "moves.h"
#include "board.h"

void initChessHeuristics(ChessBoard* board);
void closeChessHeuristics();

typedef enum {ESTIMATE, ABSOLUTE} evalType_e;
typedef enum {UN_EVAL, PRE_EVAL, FULL_EVAL} evalState_e;

typedef struct ChessHNode ChessHNode;
struct ChessHNode{
    move_t move;
    struct ChessHNode* parent;
    struct ChessHNode** children;
    int childrenCount;
    color_e toPlay;
    int inCheck;
    zob_hash_t hash;
    int halfMoveNumber;
    int depth;

    int evaluation;
    evalState_e state;
    evalType_e type;
};
ChessHNode* ChessHNode_new(ChessHNode* parent, ChessBoard* board);
void ChessHNode_delete(ChessHNode* self);
void ChessHNode_deleteChildren(ChessHNode* self);
void ChessHNode_doPreEvaluation(ChessHNode* self, ChessBoard* board);
void ChessHNode_doFullEvaluation(ChessHNode* self, ChessBoard* board);
void ChessHNode_expandBranches(ChessHNode* self, ChessMoveGenerator* gen);
void ChessHNode_expandLeaves(ChessHNode* self, ChessMoveGenerator* gen);
int ChessBoard_isInOptionalDraw(ChessBoard* board);
#endif
