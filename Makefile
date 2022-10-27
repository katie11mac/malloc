my-malloc.so: my-malloc.c
	gcc -g -Wall -pedantic -rdynamic -shared -fPIC -o my-malloc.so my-malloc.c

.PHONY: run
run: my-malloc.so
	LD_PRELOAD=./my-malloc.so ls

.PHONY: debug
debug: my-malloc.so test-malloc
	gdb --args env LD_PRELOAD=./my-malloc.so ./test-malloc

.PHONY: clean
clean:
	rm -f my-malloc my-malloc.o
