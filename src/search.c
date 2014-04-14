#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <limits.h>
#include "search.h"
#include "heuristics.h"

ChessMoveGenerator* gen;
ChessBoard* board;
int maxDepth;

void __init(ChessBoard* b){
    board = b;
    gen = ChessMoveGenerator_new(board);
}

void __close(){
    ChessMoveGenerator_delete(gen);
}

int alphabeta(ChessHNode* node, int depth, int quiecense, int deepQuiecense,
    int alpha, int beta, move_t* lineout, int* lineoutLength){
    if(depth==0){
        int delta = (node->evaluation)-(node->parent->evaluation);
        delta = delta<0?-delta:delta;
        if(delta>=100 && quiecense){
            depth++; //not quiet
            quiecense--;
        }
        else if(delta>=200 && deepQuiecense){
            depth++; //not even kindof quiet
            deepQuiecense--;
        }
        else{
            *lineoutLength = 0;
            return node->evaluation;
        }
    }
    //expand
    ChessHNode_expandBranches(node, gen);
    //if terminal
    if(node->childrenCount==0){
        ChessHNode_deleteChildren(node);
        *lineoutLength = 0;
        return node->evaluation;
    }
    //if maximizing player
    int i, eval, best=0;
    ChessHNode* child;
    move_t lines[node->childrenCount][depth+quiecense+deepQuiecense];
    int lineLengths[node->childrenCount];
    if(node->toPlay==WHITE){
        for(i=0; i<node->childrenCount; i++){
            child = node->children[i];
            ChessBoard_makeMove(board, child->move);
            eval = alphabeta(child, depth-1, quiecense, deepQuiecense, 
                alpha, beta, lines[i]+1, lineLengths+i);
            ChessBoard_unmakeMove(board);
            if(eval>alpha){
                alpha = eval;
                best = i;
            }
            if(beta <= alpha)
                break;
        }
        lineout[0] = node->children[best]->move;
        *lineoutLength = lineLengths[best]+1;
        for(i=1; i < *lineoutLength; i++)
            lineout[i] = lines[best][i];
        ChessHNode_deleteChildren(node);
        return alpha;
    }
    //minimizing player
    else{
        for(i=0; i<node->childrenCount; i++){
            child = node->children[i];
            ChessBoard_makeMove(board, child->move);
            eval = alphabeta(child, depth-1, quiecense, deepQuiecense, 
                alpha, beta, lines[i]+1, lineLengths+i);
            ChessBoard_unmakeMove(board);
            if(eval<beta){
                beta = eval;
                best = i;
            }
            if(beta <= alpha)
                break;
        }
        lineout[0] = node->children[best]->move;
        *lineoutLength = lineLengths[best]+1;
        for(i=1; i < *lineoutLength; i++)
            lineout[i] = lines[best][i];
        ChessHNode_deleteChildren(node);
        return beta;
    }
}

int getBestLine(ChessBoard* board, int depth, move_t* line, int* lineLength){
    __init(board);
    maxDepth = depth;
    ChessHNode* start = ChessHNode_new(NULL, board);
    int eval = alphabeta(start, depth, depth+1, depth, INT_MIN, INT_MAX, line, lineLength);
    ChessHNode_delete(start);
    __close();
    return eval;
}
