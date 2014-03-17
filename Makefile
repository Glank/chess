CFLAGS=-Wall -g
OBJS=board.o zobrist.o

all:	test

zobrist.o: zobrist.h zobrist.c
	$(CC) $(CFLAGS) -c zobrist.c

board.o: board.h board.c zobrist.h
	$(CC) $(CFLAGS) -c board.c

test: $(OBJS) test.c
	$(CC) $(CFLAGS) -c test.c
	$(CC) $(CFLAGS) $(OBJS) test.o -o chess_test
	valgrind --leak-check=full ./chess_test

clean:
	rm -f *.o chess_test
