CC      = gcc
override LDFLAGS = -lm -pthread
override CFLAGS += -Wall -Wextra -Werror -fwrapv -O3 -march=native

find_dragon: find_dragon.c
	make -C cubiomes native
	$(CC) $(CFLAGS) $< cubiomes/libcubiomes.a $(LDFLAGS) -o$@

clean:
	make -C cubiomes clean
	rm -f *.o find_dragon

run: find_dragon
	./find_dragon
