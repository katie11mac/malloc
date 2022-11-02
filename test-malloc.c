#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <limits.h> 

char *pointer_to_hex_le(void *ptr); // delete
char *uint64_to_string(uint64_t n);

int main(int argc, char *argv[]){

    void *address; 
    uint64_t *s;

    *s = 5; 

    // TEST 1: Unitialized heap (case 0) and requested size within the size of heap 
    // TEST 1: Unitilaized heap (case 0) and requested size outside size of heap (need to increment again) (change to 150)
    address = malloc(9); // address is 32 after starter pointer 
    write(1,"address of malloc(9):  ",sizeof("address of malloc(9):  "));
    write(1, pointer_to_hex_le(address), 16); 
    write(1,"\n\n",sizeof("\n\n"));

    memcpy(address, s, 16);
    write(1, uint64_to_string(*s), 16); //this should give us "hello"

    // write(1,"freeing our malloc(9)",sizeof("freeing our malloc(9)"));
    // write(1,"\n\n",sizeof("\n\n"));
    // free(address);

    // TEST 2: Initialized heap (case 1) and requested size within the size of heap (added at end)
    address = malloc(17); // address is 16 (size of previous malloc) + 32 (size of struct) after previous malloc (48)
    write(1,"address of malloc(17): ",sizeof("address of malloc(17): "));
    write(1, pointer_to_hex_le(address), 16); 
    write(1,"\n\n",sizeof("\n\n"));

    // TEST 3: Initialized heap (case 1) and requested size larger than size of heap 
    //          Needs to increment and add at the end 
    address = malloc(50); // address is 32 (size of previous malloc) + 32 (size of struct) after previous malloc (64)
    write(1,"address of malloc(50): ",sizeof("address of malloc(50): "));
    write(1, pointer_to_hex_le(address), 16);  
    // *** THERE IS SOMETHING WEIRD HERE BECAUSE IT DOESN'T SAY THAT IT'S INCREMENTING
    write(1,"\n\n",sizeof("\n\n"));

    // need free() to test if we place allocations in between correctly

    return 0;
}

/*
 * pointer_to_hex_le.c
 *
 * Converts a pointer (little-endian) to a printable string.
 *
 *   $ gcc -g -Wall -pedantic -o pointer_to_hex_le pointer_to_hex_le.c
 *   $ ./pointer_to_hex_le
 *   pointer_to_hex_le says: 0x000055c0fe64a2a0
 *   printf says:            0x000055c0fe64a2a0
 */

char *pointer_to_hex_le(void *ptr)
{
    static char hex[sizeof(ptr) * 2 + 1];
    char hex_chars[] = "0123456789abcdef";
    int i;
    uint8_t nibble;

    uint64_t mask = 0xf;
    uint64_t shift = 0;

    for(i=sizeof(ptr) * 2 - 1; i>=0; i-=1) {
        nibble = ((uint64_t) ptr & mask) >> shift;
        hex[i] = hex_chars[nibble];

        mask = mask << 4;
        shift += 4;
    }
    hex[sizeof(ptr) * 2] = '\0';

    return hex;
}

/*
 * uint64_to_string.c
 *
 * Converts a unsigned 64-bit integer to a printable string.
 *
 *    $ gcc -g -Wall -pedantic -o foo foo.c
 *    $ ./foo
 *    1234 is 1234
 *    4321 is 4321
 *    0 is 0
 *    2147483646 is 2147483646
 *
 */

char *uint64_to_string(uint64_t n)
{
    static char s[32] = "0000000000000000000000000000000\0";
    int i;

    for(i=30; i>0; i--) {
        s[i] = (n % 10) + '0';
        n = n / 10;
        if(n == 0)
            break;
    }

    return &s[i];
}
