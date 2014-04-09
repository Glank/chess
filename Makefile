CFLAGS=-Wall -g
OBJS=board.o zobrist.o moves.o strutl.o heuristics.o search.o
TEST_OBJS=$(OBJS)

all:    test

search.o: search.h search.c heuristics.h moves.h board.h
	$(CC) $(CFLAGS) -c search.c

heuristics.o: heuristics.h heuristics.c moves.h board.h
	$(CC) $(CFLAGS) -c heuristics.c

strutl.o: strutl.h strutl.c
	$(CC) $(CFLAGS) -c strutl.c

moves.o: moves.h moves.c board.h
	$(CC) $(CFLAGS) -c moves.c

zobrist.o: zobrist.h zobrist.c
	$(CC) $(CFLAGS) -c zobrist.c

board.o: board.h board.c zobrist.h strutl.h
	$(CC) $(CFLAGS) -c board.c

test: $(TEST_OBJS) test.c
	$(CC) $(CFLAGS) -c test.c
	$(CC) $(CFLAGS) $(TEST_OBJS) test.o -o test
	valgrind --leak-check=full ./test
	rm test

clean:
	rm -f *.o test
