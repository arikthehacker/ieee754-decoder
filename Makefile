CC = gcc
CFLAGS = -Wall -Wshadow -Wunreachable-code -Wredundant-decls -Wmissing-declarations -Wold-style-definition -Wmissing-prototypes -Wdeclaration-after-statement -Werror -Wextra -Wpedantic -Wno-return-local-addr -Wunsafe-loop-optimizations -Wuninitialized

all: hex-2-float float-2-hex

hex-2-float: hex-2-float.o
	$(CC) $(CFLAGS) -o $@ $^ -lm

hex-2-float.o: hex-2-float.c
	$(CC) $(CFLAGS) -c $<

float-2-hex: float-2-hex.o
	$(CC) $(CFLAGS) -o $@ $^ -lm

float-2-hex.o: float-2-hex.c
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f *.o hex-2-float float-2-hex

cls: clean

run: hex-2-float
	echo 0x40490FDB | ./hex-2-float

test:
	./test.sh

.PHONY: all clean cls run test
