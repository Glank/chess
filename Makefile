CFLAGS=-Wall -g
OBJS=board.o zobrist.o moves.o ll

all:    test	

moves.o: moves.h moves.c board.h
	$(CC) $(CFLAGS) -c moves.c

zobrist.o: zobrist.h zobrist.c
	$(CC) $(CFLAGS) -c zobrist.c

board.o: board.h board.c zobrist.h
	$(CC) $(CFLAGS) -c board.c

test: $(OBJS) test.c
	$(CC) $(CFLAGS) -c test.c
	$(CC) $(CFLAGS) $(OBJS) test.o -o test
	valgrind --leak-check=full ./test

clean:
	rm -f *.o chess_test
