CFLAGS=-Wall -g
OBJS=board.o zobrist.o moves.o strutl.o
TEST_OBJS=$(OBJS)

all:    test	

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

clean:
	rm -f *.o test
