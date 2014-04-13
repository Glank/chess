CFLAGS=-Wall -g
SRC=src
TEST_SRC=src
BLD=build
OBJS=$(BLD)/board.o
OBJS+=$(BLD)/zobrist.o
OBJS+=$(BLD)/moves.o
OBJS+=$(BLD)/strutl.o
OBJS+=$(BLD)/heuristics.o
OBJS+=$(BLD)/search.o
OBJS+=$(BLD)/narrator.o

all: test

$(BLD):
	mkdir build

$(BLD)/narrator.o: $(BLD) $(SRC)/narrator.h $(SRC)/narrator.c $(SRC)/moves.h $(SRC)/board.h
	$(CC) $(CFLAGS) -c $(SRC)/narrator.c -o $(BLD)/narrator.o

$(BLD)/search.o: $(BLD) $(SRC)/search.h $(SRC)/search.c $(SRC)/heuristics.h $(SRC)/moves.h $(SRC)/board.h
	$(CC) $(CFLAGS) -c $(SRC)/search.c -o $(BLD)/search.o

$(BLD)/heuristics.o: $(BLD) $(SRC)/heuristics.h $(SRC)/heuristics.c $(SRC)/moves.h $(SRC)/board.h
	$(CC) $(CFLAGS) -c $(SRC)/heuristics.c -o $(BLD)/heuristics.o

$(BLD)/strutl.o: $(BLD) $(SRC)/strutl.h $(SRC)/strutl.c
	$(CC) $(CFLAGS) -c $(SRC)/strutl.c -o $(BLD)/strutl.o

$(BLD)/moves.o: $(BLD) $(SRC)/moves.h $(SRC)/moves.c $(SRC)/board.h
	$(CC) $(CFLAGS) -c $(SRC)/moves.c -o $(BLD)/moves.o

$(BLD)/zobrist.o: $(BLD) $(SRC)/zobrist.h $(SRC)/zobrist.c
	$(CC) $(CFLAGS) -c $(SRC)/zobrist.c -o $(BLD)/zobrist.o

$(BLD)/board.o: $(BLD) $(SRC)/board.h $(SRC)/board.c $(SRC)/zobrist.h $(SRC)/strutl.h
	$(CC) $(CFLAGS) -c $(SRC)/board.c -o $(BLD)/board.o

test: $(BLD) $(OBJS) $(TEST_SRC)/test.c
	$(CC) $(CFLAGS) -c $(SRC)/test.c -o $(BLD)/test.o
	$(CC) $(CFLAGS) $(OBJS) $(BLD)/test.o -o test
	valgrind --leak-check=full ./test
	rm test

chess: $(BLD) $(OBJS) $(TEST_SRC)/chess.c
	$(CC) $(CFLAGS) -c $(SRC)/chess.c -o $(BLD)/chess.o
	$(CC) $(CFLAGS) $(OBJS) $(BLD)/chess.o -o chess

clean:
	rm -f build/* 
	rm -f test
	rm -f chess
