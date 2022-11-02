my-malloc.so: my-malloc.c
	gcc -g -Wall -pedantic -rdynamic -shared -fPIC -o my-malloc.so my-malloc.c

test-malloc: test-malloc.c my-malloc.so
	gcc -Wall -pedantic -o test-malloc test-malloc.c

.PHONY: run_ls
run_ls: my-malloc.so
	LD_PRELOAD=./my-malloc.so ls

.PHONY: debug
debug: my-malloc.so test-malloc
	gdb --args env LD_PRELOAD=./my-malloc.so ./test-malloc

.PHONY: clean
clean:
	rm -f my-malloc my-malloc.o
