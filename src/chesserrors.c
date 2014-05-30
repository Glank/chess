#include <string.h>
#include <stdio.h>
#include <assert.h>
#include "chesserrors.h"
chessError_e globalError = NO_ERROR;
char globalDescription[MAX_ERROR_DESCRIPTION_LENGTH] = "";
void setError(chessError_e error, const char* description){
    globalError = error;
    strcpy(globalDescription, description);
}
chessError_e getErrorType(){
    return globalError;
}
const char* getErrorDescription(){
    return globalDescription;
}
void printError(){
    switch(globalError){
    NO_ERROR: printf("Invalid Error Handeling.\n"); exit(1);
    PARSING_ERROR: printf("Parsing Error: "); break;
    }
    printf("%s\n", globalDescription);
}
void printErrorAndDie(){
    printError();
    exit(1);
}
