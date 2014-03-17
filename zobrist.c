#include <stdlib.h>
#include "zobrist.h"

void initZobrist(){
    ZOBRIST_TABLE = (zob_hash_t*)malloc(
        sizeof(zob_hash_t)*ZOB_TABLE_SIZE
    );
    srand(12345);
    int i,b;
    for(i=0; i<ZOB_TABLE_SIZE; i++){
        zob_hash_t newHash = 0;
        for(b=0; b<sizeof(zob_hash_t); b++)
            newHash = (newHash<<8)+(rand()%256);
        ZOBRIST_TABLE[i] = newHash;
    }
}

void closeZobrist(){
    free(ZOBRIST_TABLE);
}
