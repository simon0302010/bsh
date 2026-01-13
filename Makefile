CC = g++
CFLAGS = -Wall -O2
LDFLAGS = -lfmt
TARGET = bsh

all: $(TARGET)
	@make clean-obj

$(TARGET): main.o command.o utils.o
	$(CC) $(CFLAGS) -o $(TARGET) main.o command.o utils.o $(LDFLAGS)

main.o: main.cpp command.h utils.h
	$(CC) $(CFLAGS) -c main.cpp

command.o: command.cpp command.h utils.h
	$(CC) $(CFLAGS) -c command.cpp

utils.o: utils.cpp utils.h
	$(CC) $(CFLAGS) -c utils.cpp

clean-obj:
	rm -f *.o

clean:
	rm -f *.o $(TARGET)

.PHONY: all clean
