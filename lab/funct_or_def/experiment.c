#include <stdio.h>
#include <time.h>
#include <inttypes.h>
#include <assert.h>

#define trials 10000000000L
#define inc_definition(in) (1664525*in+1013904223)
uint32_t inc_function(uint32_t in){return 1664525*in+1013904223;}

/*
### Results ###
c -Wall -g -O3 experiment.c -o experiment
./experiment
 */

int main(void){
    clock_t start, end;
    double delta;
    long i;
    uint32_t rand;

    start = clock();
    rand = 0;
    for(i=0; i < trials; i++)
        rand = inc_definition(rand);
    end = clock();
    delta = (double)(end-start)/CLOCKS_PER_SEC;
    printf("Result: %d\n", rand);
    printf("Function: %f seconds\n", delta);

    start = clock();
    rand = 0;
    for(i=0; i < trials; i++)
        rand = inc_definition(rand);
    end = clock();
    delta = (double)(end-start)/CLOCKS_PER_SEC;
    printf("Result: %d\n", rand);
    printf("Define: %f seconds\n", delta);

    return 0;
}
