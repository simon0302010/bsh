CC = g++
CFLAGS = -Wall -lfmt -O2

all:
	$(CC) $(CFLAGS) -o bsh main.cpp

run:
	$(CC) $(CFLAGS) -o bsh main.cpp && ./bsh

clean:
	rm -f bsh
