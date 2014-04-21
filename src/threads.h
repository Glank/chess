#ifndef CHESS_THREADS_H_INCLUDE
#define CHESS_THREADS_H_INCLUDE

//Wrappers for operating system dependent multithreading implementations

typedef enum {NEW_THREAD, RUNNING_THREAD, COMPLETED_THREAD} threadState_e;

typedef struct ChessThread ChessThread;
typedef struct ChessMutex ChessMutex;

ChessThread* ChessThread_new(void*(*run_function)(void*));
void ChessThread_delete(ChessThread* self);
int ChessThread_start(ChessThread* self, void* args);
int ChessThread_join(ChessThread* self, void** retval);
int ChessThread_sleep(long milliseconds);
threadState_e ChessThread_getState(ChessThread* self);

ChessMutex* ChessMutex_new();
void ChessMutex_delete(ChessMutex* self);
void ChessMutex_lock(ChessMutex* self);
void ChessMutex_unlock(ChessMutex* self);

#endif
