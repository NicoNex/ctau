TARGET = bench
CFLAGS = -Werror -Wall -Isrc/ -g -fno-gcse -O3 -march=native -mtune=native
SRC_FILES = src/ast/*.c src/code/*.c src/compiler/*.c src/data/*.c src/item/*.c src/lexer/*.c src/obj/*.c src/parser/*.c src/vm/*.c
FILES = main.c $(SRC_FILES)
TEST_FILES = tests/tautest.c $(SRC_FILES)

all:
	gcc $(CFLAGS) -o $(TARGET) $(FILES)

clean:
	rm -f $(TARGET)

test:
	gcc $(CFLAGS) -o ctau_test $(TEST_FILES)
	./ctau_test
	rm -f ctau_test

tests: test

.PHONY: all clean
