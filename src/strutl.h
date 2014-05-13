#ifndef STRUTL_H_INCLUDE
#define STRUTL_H_INCLUDE
int isDigit(char c);
int isWhiteSpace(char c);
int digitToInt(char digit);
int getCharIndex(char c, char* str, int len);
char* getLine();
char* str_unload_buffer(char* buffer, char* out, 
    int* outLen, int* outMaxLen);
#endif
