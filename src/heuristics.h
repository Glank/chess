#ifndef CHESS_HEURISTICS_H_INCLUDE
#define CHESS_HEURISTICS_H_INCLUDE
#include "moves.h"
#include "board.h"

typedef enum {ESTIMATE, ABSOLUTE} evalType_e;

typedef struct ChessHEngine ChessHEngine;
typedef struct ChessHNode ChessHNode;

//TODO: make this private
struct ChessHEngine{
    ChessMoveGenerator* expGen;
    ChessMoveGenerator* evalGen;
    ChessBoard* board;
};
ChessHEngine* ChessHEngine_new(ChessBoard* board);
void ChessHEngine_delete(ChessHEngine* self);

//TODO: make this private
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
    evalType_e type;
};
ChessHNode* ChessHNode_new(ChessHNode* parent, ChessHEngine* engine);
void ChessHNode_delete(ChessHNode* self);
void ChessHNode_deleteChildren(ChessHNode* self);
void ChessHNode_evaluate(ChessHNode* self, ChessHEngine* engine);
void ChessHNode_expand(ChessHNode* self, ChessHEngine* engine);
#endif
