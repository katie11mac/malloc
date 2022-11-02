make test-malloc
make
LD_PRELOAD=./my-malloc.so ./test-malloc