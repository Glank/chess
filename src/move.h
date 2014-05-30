//encoding for a chess move is specified here:
//https://chessprogramming.wikispaces.com/Encoding+Moves
//first 2 bits are the capture and pawn promotion flags,
//the next 2 bits are used in conjunction with the first two
//to identify the special move type,
//next 6 are the from location,
//last 6 are the to location
// CP SS FFFFFF TTTTTT
#include <inttypes.h>
typedef uint8_t location_t;
#define UNKNOWN_LOCATION 255
#define RANK_FILE(r,f) (((r)<<3)|(f))
#define GET_RANK(loc) ((loc)>>3)
#define GET_FILE(loc) ((loc)&7)
typedef uint16_t move_t;
#define NEW_MOVE(from,to) ((from<<6)|to)
#define GET_FROM(move) ((move>>6)&63)
#define GET_TO(move) (move&63)
#define GET_META(move) (move&0xF000)
#define CAPTURE_MOVE_FLAG 0x4000
#define PROMOTION_MOVE_FLAG 0x8000
//special moves
#define DOUBLE_PAWN_PUSH_MOVE 0x1000
#define KING_CASTLE_MOVE 0x2000
#define QUEEN_CASTLE_MOVE 0x3000
#define EN_PASSANT_MOVE 0x5000
//promotion types (must recognize flags first)
#define KNIGHT_PROMOTION 0x0000
#define BISHOP_PROMOTION 0x1000
#define ROOK_PROMOTION 0x2000
#define QUEEN_PROMOTION 0x3000
#define PROMOTION_MASK 0x3000
