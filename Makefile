TARGET = bench
CFLAGS = -Werror -Wall -Isrc/ -g -fno-gcse -O3 -march=native -mtune=native
FILES = main.c src/ast/*.c src/code/*.c src/compiler/*.c src/data/*.c src/item/*.c src/lexer/*.c src/obj/*.c src/parser/*.c src/vm/*.c

all:
	gcc $(CFLAGS) -o $(TARGET) $(FILES)

clean:
	rm -f $(TARGET)

.PHONY: all clean

