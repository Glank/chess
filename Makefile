CFLAGS=-Wall -g
OBJS=board.o zobrist.o moves.o
TEST_OBJS=board.o zobrist.o

all:    test	

moves.o: moves.h moves.c board.h
	$(CC) $(CFLAGS) -c moves.c

zobrist.o: zobrist.h zobrist.c
	$(CC) $(CFLAGS) -c zobrist.c

board.o: board.h board.c zobrist.h
	$(CC) $(CFLAGS) -c board.c

test: $(TEST_OBJS) test.c
	$(CC) $(CFLAGS) -c test.c
	$(CC) $(CFLAGS) $(TEST_OBJS) test.o -o test
	valgrind --leak-check=full ./test

clean:
	rm -f *.o test
