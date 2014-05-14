#include "mind.h"
struct ChessMind{
    OpeningBook* book;
    TTable* table;
    SearchThread* thread;
    int timeout;
};
