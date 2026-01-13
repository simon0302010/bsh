CC = g++
CFLAGS = -Wall -O3
LDFLAGS = -lfmt
TARGET = bsh
SRCDIR = src
OBJDIR = obj

SRC = $(SRCDIR)/main.cpp $(SRCDIR)/command.cpp $(SRCDIR)/utils.cpp
OBJ = $(OBJDIR)/main.o $(OBJDIR)/command.o $(OBJDIR)/utils.o

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJ) $(LDFLAGS)

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	@mkdir -p $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJDIR) $(TARGET)

.PHONY: all clean
