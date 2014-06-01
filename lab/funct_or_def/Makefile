CFLAGS=-Wall -g -O3

experiment: experiment.c
	$(CC) $(CFLAGS) experiment.c -o experiment

test: experiment
	./experiment
	rm experiment

clean:
	rm -f experiment
