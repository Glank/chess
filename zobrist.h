#ifndef ZOBRIST_H_INCLUDE
#define ZOBRIST_H_INCLUDE
#include <inttypes.h>
#define ZOB_WHITE_KING_CASTLE 768
#define ZOB_WHITE_QUEEN_CASTLE 769
#define ZOB_BLACK_KING_CASTLE 770
#define ZOB_BLACK_QUEEN_CASTLE 771
#define ZOB_EN_PASSANT_START 772
#define ZOB_TO_PLAY 780
#define ZOB_TABLE_SIZE 781
typedef uint32_t zob_hash_t;
zob_hash_t* ZOBRIST_TABLE;
void initZobrist();
void closeZobrist();
#endif
