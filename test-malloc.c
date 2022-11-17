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

/*
* Test for my-malloc.c 
* 
* 
* Set Up: 
*   - INCREMENT_BY 160
*   - Use gdb
*   - Add break points at functions being called
*   - Add break points after each assignment
*/
int main(int argc, char *argv[])
{
    void *address, *address2, *address3, *address4, *address5, *address6;




    write(1,"-----TESTING MALLOC-----\n",sizeof("-----TESTING MALLOC-----\n"));
    // TEST 1: Unitialized heap (case 0) and requested size within the size of heap 
    address = malloc(9); 
    // EXPECTED RESULTS: address = heap_begin_ptr + 32 (size of struct)
    write(1,"address  (malloc 9) : ",sizeof("address (malloc 9) : "));
    write(1, pointer_to_hex_le(address), 16);
    write(1,"\n",sizeof("\n"));

    // TEST 2: Initialized heap (case 1) and requested size within the size of heap (added at end)
    address2 = malloc(17); 
    // EXPECTED RESULTS: address2 = address + 16 (size of prev malloc) + 32 (size of struct)   
    //                            = address + 48  
    write(1,"address2 (malloc 17): ",sizeof("address2 (malloc 17): "));
    write(1, pointer_to_hex_le(address2), 16);
    write(1,"\n",sizeof("\n"));

    // TEST 3: Initialized heap (case 1) and requested size larger than size of heap 
    //         Needs to increment and add at the end 
    address3 = malloc(50); 
    // EXPECTED RESULTS: address3 = address2 + 32 (size of prev malloc) + 32 (size of struct)
    //                            = address2 + 64 
    write(1,"address3 (malloc 50): ",sizeof("address2 (malloc 50): "));
    write(1, pointer_to_hex_le(address3), 16);
    write(1,"\n",sizeof("\n"));

    address4 = malloc(32);
    // EXPECTED RESULTS: address4 = address3 + 64 (size of prev malloc) + 32 (size of struct)
    //                            = address3 + 96 
    write(1,"address4 (malloc 32): ",sizeof("address2 (malloc 32): "));
    write(1, pointer_to_hex_le(address4), 16);
    write(1,"\n",sizeof("\n"));

    write(1,"\n",sizeof("\n"));

    


    
    write(1,"-----TESTING FREE + MALLOC-----\n",sizeof("-----TESTING FREE + MALLOC-----\n"));
    
    // TEST 4: Free the first struct (address), malloc size too big for beginning gap, malloc size to fit in begining gap
    write(1,"FREEING address\n",sizeof("FREEING address\n"));
    free(address);

    // malloc size too big for beginning gap (beginning gap can hold up to 16)
    address5 = malloc(20); 
    // EXPECTED RESULTS: address5 = address4 + 32 (size of prev malloc) + 32 (size of struct)
    //                            = address4 + 64
    write(1,"address5 (malloc 20): ",sizeof("address5 (malloc 20): "));
    write(1, pointer_to_hex_le(address5), 16); 
    write(1,"\n",sizeof("\n"));
    
    // malloc size that can fit in beginning gap (beginning gap can hold up to 16)
    address = malloc(11);
    write(1,"address  (malloc 11): ",sizeof("address  (malloc 11): "));
    write(1, pointer_to_hex_le(address), 16);
    write(1,"\n",sizeof("\n"));
    // EXPECTED RESULTS: address = original address
    write(1,"\n",sizeof("\n"));






    write(1,"-----TESTING FREE + MALLOC w/ REALLOC-----\n",sizeof("-----TESTING FREE + MALLOC w/ REALLOC-----\n"));

    // TEST 5: Free a middle struct, malloc too big for the gap, malloc to fit in the gap
    write(1,"FREEING address2\n",sizeof("FREEING address2\n"));
    realloc(address2, 0);

    // malloc size too big for gap (gap can hold up to 32)
    address6 = malloc(34); 
    // EXPECTED RESULTS: address6 = address5 + 32 (size of prev malloc) + 32 (size of struct)
    //                            = address5 + 64
    write(1,"address6 (malloc 34): ",sizeof("address6 (malloc 34): "));
    write(1, pointer_to_hex_le(address6), 16); 
    write(1,"\n",sizeof("\n"));

    // realloc on NULL, so malloc size that can fit in gap (gap can hold up to 32)
    address2 = realloc(NULL, 11);
    write(1,"address2 (malloc 20): ",sizeof("address  (malloc 20): "));
    write(1, pointer_to_hex_le(address2), 16);
    write(1,"\n",sizeof("\n"));
    // EXPECTED RESULTS: address = original address2

    



    write(1,"-----TESTING REALLOC-----\n",sizeof("-----TESTING REALLOC-----\n"));    


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
