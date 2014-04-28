#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "strutl.h"
int isDigit(char c){
    return (c<='9') && (c>='0');
}
int digitToInt(char digit){
    assert(isDigit(digit));
    return digit-'0';
}
int getCharIndex(char c, char* str, int len){
    int i;
    for(i = 0; i < len; i++){
        if(str[i]==c){
            return i;
        }
    }
    return -1;
}
char* getLine() {
    char * line = malloc(100), * linep = line;
    size_t lenmax = 100, len = lenmax;
    int c;
    if(line == NULL)
        return NULL;
    for(;;){
        c = fgetc(stdin);
        if(c == EOF)
            break;
        if(--len == 0) {
            len = lenmax;
            char * linen = realloc(linep, lenmax *= 2);
            if(linen == NULL) {
                free(linep);
                return NULL;
            }
            line = linen + (line - linep);
            linep = linen;
        }
        if((*line++ = c) == '\n'){
            line--;
            break;
        }
    }
    *line = '\0';
    return linep;
}
