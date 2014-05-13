#ifndef STRUTL_H_INCLUDE
#define STRUTL_H_INCLUDE
#include <stdio.h>
int isDigit(char c);
int isWhiteSpace(char c);
int isBlankLine(char* line);
int digitToInt(char digit);
int getCharIndex(char c, char* str, int len);
char* getLine();
char* fgetLine(FILE* fp);
char* str_unload_buffer(char* buffer, char* out, 
    int* outLen, int* outMaxLen);
#endif
