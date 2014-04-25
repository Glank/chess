#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <assert.h>
#include "threads.h"

struct ChessThread{
    pthread_t thread;
    pthread_attr_t attributes;
    void*(*run_function)(void*);
    void* args;
    threadState_e state;
};
ChessThread* ChessThread_new(void*(*run_function)(void*)){
    ChessThread* self = (ChessThread*)malloc(sizeof(ChessThread));
    self->run_function = run_function;
    self->state = NEW_THREAD;
    pthread_attr_init(&(self->attributes));
    pthread_attr_setdetachstate(&(self->attributes), PTHREAD_CREATE_JOINABLE);
    return self;
}
void ChessThread_delete(ChessThread* self){
    pthread_attr_destroy(&(self->attributes));
    free(self);
}
void* runWrapper(void* args){
    ChessThread* thread = (ChessThread*)args;
    thread->state = RUNNING_THREAD;
    void* ret = (*thread->run_function)(thread->args);
    thread->state = COMPLETED_THREAD;
    return ret;
}
int ChessThread_start(ChessThread* self, void* args){
    assert(self->state==NEW_THREAD);
    self->args = args;
    return pthread_create(&self->thread, &(self->attributes), &runWrapper, self);
}
int ChessThread_join(ChessThread* self, void** retval){
    return pthread_join(self->thread, retval);
}
int ChessThread_sleep(long milliseconds){
    int seconds = (int)(milliseconds/1000);
    useconds_t microseconds = 1000*(useconds_t)(milliseconds%1000);
    int ret = sleep(seconds);
    if(ret)
        return ret;
    ret = usleep(microseconds);
    return ret;
}
threadState_e ChessThread_getState(ChessThread* self){
    return self->state;
}

struct ChessMutex{
    pthread_mutex_t mutex;
};
ChessMutex* ChessMutex_new(){
    ChessMutex* self = (ChessMutex*)malloc(sizeof(ChessMutex));
    pthread_mutex_init(&(self->mutex), NULL);
    return self;
}
void ChessMutex_delete(ChessMutex* self){
    pthread_mutex_destroy(&(self->mutex));
    free(self);
}
void ChessMutex_lock(ChessMutex* self){
    pthread_mutex_lock(&(self->mutex));
}
void ChessMutex_unlock(ChessMutex* self){
    pthread_mutex_unlock(&(self->mutex));
    ChessThread_sleep(1);
}
