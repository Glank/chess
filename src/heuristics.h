#ifndef CHESS_HEURISTICS_H_INCLUDE
#define CHESS_HEURISTICS_H_INCLUDE
#include "moves.h"
#include "board.h"
#define TTABLE_SIZE 2048

void initChessHeuristics(ChessBoard* board);
void closeChessHeuristics();

typedef enum {ESTIMATE, ABSOLUTE} evalType_e;

typedef struct ChessHEngine ChessHEngine;
typedef struct ChessHNode ChessHNode;
typedef struct TNode TNode;
typedef struct TTable TTable;

struct ChessHEngine{
    ChessMoveGenerator* expGen;
    ChessMoveGenerator* evalGen;
    ChessBoard* board;
    TTable* table;
};
ChessHEngine* ChessHEngine_new(ChessBoard* board);
void ChessHEngine_delete(ChessHEngine* self);

struct TNode{
    zob_hash_t hash;
    int halfMoveNumber;
    int depth;
    int evaluation;
    evalType_e type;
};

struct TTable{
    TNode nodes[TTABLE_SIZE];
    int minHalfMoveNumber;
};
TTable* TTable_new();
void TTable_delete(TTable* self);

struct ChessHNode{
    move_t move;
    struct ChessHNode* parent;
    struct ChessHNode** children;
    int childrenCount;
    color_e toPlay;
    int inCheck;
    TNode info;
};
ChessHNode* ChessHNode_new(ChessHNode* parent, ChessHEngine* engine);
void ChessHNode_delete(ChessHNode* self);
void ChessHNode_deleteChildren(ChessHNode* self);
void ChessHNode_evaluate(ChessHNode* self, ChessHEngine* engine);
void ChessHNode_expand(ChessHNode* self, ChessHEngine* engine);
#endif
