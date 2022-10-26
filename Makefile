my-malloc: my-malloc.o
	gcc -g -Wall -pedantic -rdynamic -shared -fPIC -o my-malloc.so my-malloc.c

LD_PRELOAD=./my-malloc.so ls

gdb --args env LD_PRELOAD=./my-malloc.so ./test-malloc

.PHONY: clean
clean:
	rm -f my-malloc my-malloc.o
