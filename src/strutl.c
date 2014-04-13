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

