TARGET = bin/main

SRCS = main.c

all: $(TARGET)

$(TARGET): $(SRCS)
	@mkdir -p bin
	gcc $(SRCS) -o $(TARGET)

clean:
	rm -rf bin
