CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -I.
SRC = src/host.c src/router.c main.c
OBJ = $(SRC:.c=.o)
TARGET = bin/main

all: $(TARGET)

$(TARGET): $(OBJ)
	@mkdir -p bin
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJ)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f src/*.o *.o $(TARGET)
