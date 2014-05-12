#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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
char* str_unload_buffer(char* buffer, char* out, 
    int* outLen, int* outMaxLen){
    int bufferLen = strlen(buffer);
    while(bufferLen+(*outLen)>(*outMaxLen)){
        //expand out to twice of it's old max size
        char* newOut = (char*)
            malloc(2*((*outMaxLen)+1));
        (*outMaxLen) = 2*((*outMaxLen)+1)-1;
        strcpy(newOut, out);
        free(out);
        out = newOut;
    }
    strcpy(out+(*outLen), buffer);
    buffer[0] = '\0';
    (*outLen)+=bufferLen;
    return out;
}
