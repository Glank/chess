#ifndef CHESS_ERROR_H_INCLUDE
#define CHESS_ERROR_H_INCLUDE
#define MAX_ERROR_DESCRIPTION_LENGTH 1024
typedef enum {
    NO_ERROR, PARSING_ERROR
} chessError_e;
void setError(chessError_e error, const char* description);
chessError_e getErrorType();
const char* getErrorDescription();
void printError();
void printErrorAndDie();
#endif
