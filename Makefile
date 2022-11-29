CFLAGS=-Wall -pedantic -g

my-malloc.so: my-malloc.c
	gcc $(CFLAGS) -rdynamic -shared -fPIC -o $@ $^

test-malloc: test-malloc.c my-malloc.so
	gcc $(CFLAGS) -o $@ test-malloc.c

.PHONY: run_ls
run_ls: my-malloc.so
	LD_PRELOAD=./my-malloc.so ls

.PHONY: debug
debug: my-malloc.so test-malloc
	gdb --args env LD_PRELOAD=./my-malloc.so ./test-malloc

.PHONY: clean
clean:
	rm -f my-malloc.so test-malloc
