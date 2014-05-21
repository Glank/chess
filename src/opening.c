#include "opening.h"
#include <stdlib.h>
#include <assert.h>
#include <errno.h>

//PRIVATE
typedef struct OpeningNode OpeningNode;

struct OpeningBook{
    int sourceGames;
    OpeningNode* root;
};

struct OpeningNode{
    move_t move;
    int gamesCount;
    OpeningNode** children;
    int childrenCount;
    int maxChildrenCount;
};
OpeningNode* OpeningNode_new(){
    OpeningNode* self = (OpeningNode*)malloc(sizeof(OpeningNode));
    self->move = 0;
    self->gamesCount = 0;
    self->children = (OpeningNode**)malloc(sizeof(OpeningNode*)*4);
    self->childrenCount = 0;
    self->maxChildrenCount = 4;
    return self;
}
void OpeningNode_delete(OpeningNode* self){
    assert(self!=NULL);
    int i;
    for(i=0; i<self->childrenCount; i++)
        OpeningNode_delete(self->children[i]);
    if(self->children!=NULL)
        free(self->children);
    free(self);
}
void OpeningNode_makeRoom(OpeningNode* self){
    int newMax = self->maxChildrenCount?self->maxChildrenCount*2:1;
    OpeningNode** newChildren = (OpeningNode**)
        malloc(sizeof(OpeningNode*)*newMax);
    int i;
    for(i = 0; i < self->childrenCount; i++)
        newChildren[i] = self->children[i];
    self->maxChildrenCount = newMax;
    free(self->children);
    self->children = newChildren;
}
void OpeningNode_expand(OpeningNode* self, MoveIterator* iterator,
    int depth){
    self->gamesCount++;
    if(!MoveIterator_hasNext(iterator) || depth==0)
        return;
    move_t move = MoveIterator_getNext(iterator);
    int i;
    for(i=0; i<self->childrenCount; i++){
        if(self->children[i]->move==move)
            break;
    }
    if(i==self->childrenCount){
        //make a new child
        if(self->childrenCount==self->maxChildrenCount)
            OpeningNode_makeRoom(self);
        self->children[i] = OpeningNode_new();
        self->children[i]->move = move;
        self->childrenCount++;
    }
    OpeningNode_expand(self->children[i], iterator, depth-1);
}
void OpeningNode_write(OpeningNode* self, FILE* fp){
    assert(self!=NULL);
    int writen;
    writen = fwrite(&(self->move), sizeof(move_t), 1, fp);
    assert(writen==1);
    writen = fwrite(&(self->gamesCount), sizeof(int), 1, fp);
    assert(writen==1);
    writen = fwrite(&(self->childrenCount), sizeof(int), 1, fp);
    assert(writen==1);
    int i;
    for(i = 0; i < self->childrenCount; i++){
        OpeningNode_write(self->children[i], fp);
    }
}
OpeningNode* OpeningNode_read(FILE* fp){
    OpeningNode* self = (OpeningNode*)malloc(sizeof(OpeningNode));
    int read;
    read = fread(&(self->move), sizeof(move_t), 1, fp);
    assert(read==1);
    read = fread(&(self->gamesCount), sizeof(int), 1, fp);
    assert(read==1);
    read = fread(&(self->childrenCount), sizeof(int), 1, fp);
    assert(read==1);
    self->maxChildrenCount = self->childrenCount;
    if(self->childrenCount==0)
        self->children = NULL;
    else{
        self->children = (OpeningNode**)
            malloc(sizeof(OpeningNode*)*self->childrenCount);
        int i;
        for(i = 0; i < self->childrenCount; i++)
            self->children[i] = OpeningNode_read(fp);
    }
    return self;
}
OpeningNode* OpeningNode_lookupLeaf(OpeningNode* self, move_t* moves,
    int length){
    if(length==0)
        return self;
    move_t move = moves[0];
    int i;
    for(i=0; i < self->childrenCount; i++){
        if(self->children[i]->move==move)
            break;
    }
    if(i==self->childrenCount)
        return NULL;
    OpeningNode* child = self->children[i];
    return OpeningNode_lookupLeaf(child, moves+1, length-1);
}
void OpeningNode_print(OpeningNode* self, ChessBoard* board, 
    int depth, int minGames){
    if(self->gamesCount<minGames)
        return;
    int i;
    for(i=0;i<depth;i++)
        printf("\t");
    if(self->move==0)
        board = ChessBoard_new(FEN_START);
    else{
        char out[10];
        int outSize;
        toAlgebraicNotation(self->move, board, out, &outSize);
        printf("%d) %s\n", self->gamesCount, out);
        ChessBoard_makeMove(board, self->move);
    }
    for(i=0;i<self->childrenCount;i++)
        OpeningNode_print(self->children[i], board, depth+1, minGames);
    if(self->move==0)
        ChessBoard_delete(board);
    else
        ChessBoard_unmakeMove(board);
}

//public
void OpeningBook_generate(char* sourceName, char* outName,
    int maxDepth){
    OpeningNode* root = OpeningNode_new();
    //compile
    FILE* fp = fopen(sourceName, "r");
    int eofFlag = 0;
    PGNRecord* pgn = PGNRecord_newFromFile(fp, &eofFlag);
    int games = 0;
    //read until EOF
    while(!eofFlag){
        if(pgn!=NULL){
            games++;
            MoveIterator* iterator = PGNRecord_getMoveIterator(pgn);
            OpeningNode_expand(root, iterator, maxDepth);
            MoveIterator_delete(iterator);
            PGNRecord_delete(pgn);
        }
        pgn = PGNRecord_newFromFile(fp, &eofFlag);
    }
    fclose(fp);
    //save
    fp = fopen(outName, "w");
    int writen;
    writen = fwrite(&games, sizeof(int), 1, fp);
    assert(writen==1);
    OpeningNode_write(root, fp);
    fclose(fp);
    //clean up
    OpeningNode_delete(root);
}
OpeningBook* OpeningBook_load(char* fileName){
    FILE* fp = fopen(fileName, "r");
    if(fp==NULL)
        return NULL;
    OpeningBook* self = (OpeningBook*)malloc(sizeof(OpeningBook));
    int read;
    read = fread(&(self->sourceGames), sizeof(int), 1, fp);
    assert(read==1);
    self->root = OpeningNode_read(fp);
    fclose(fp);
    return self;
}
void OpeningBook_delete(OpeningBook* self){
    OpeningNode_delete(self->root);
    free(self);
}
void OpeningBook_print(OpeningBook* self, int minGames){
    printf("From %d games:\n", self->sourceGames);
    OpeningNode_print(self->root, NULL, -1, minGames);
}
int OpeningBook_hasLine(OpeningBook* self, ChessBoard* board,
    int minGames){
    GameInfo* info = (GameInfo*)board->extra;
    OpeningNode* leaf = OpeningNode_lookupLeaf(self->root,
        info->moves, info->movesCount);
    if(leaf==NULL || leaf->gamesCount<minGames)
        return 0;
    //assure that there is a child with a valid line
    int i;
    for(i = 0; i < leaf->childrenCount; i++){
        if(leaf->children[i]->gamesCount>=minGames)
            break;
    }
    return i!=leaf->childrenCount;
}
move_t OpeningBook_randNextMove(OpeningBook* self, ChessBoard* board,
    int minGames){
    GameInfo* info = (GameInfo*)board->extra;
    OpeningNode* leaf = OpeningNode_lookupLeaf(self->root,
        info->moves, info->movesCount);
    assert(leaf!=NULL);
    int* childrenOrder = (int*)malloc(sizeof(int)*leaf->childrenCount);
    int i,j,temp;
    for(i=0; i<leaf->childrenCount;i++)
        childrenOrder[i] = i;
    for(i=0; i<leaf->childrenCount-1;i++){
        j = rand()%(leaf->childrenCount-i);
        temp = childrenOrder[j];
        childrenOrder[j] = childrenOrder[i];
        childrenOrder[i] = temp;
    }
    for(i=0; i<leaf->childrenCount; i++){
        j = childrenOrder[i];
        if(leaf->children[j]->gamesCount>=minGames)
            break;
    }
    assert(i!=leaf->childrenCount);
    free(childrenOrder);
    return leaf->children[j]->move;
}
