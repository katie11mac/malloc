/*
* my-malloc.c
*/

#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>

#define INCREMENT_BY 48
static void * begin_ptr = NULL; // starts as NULL so know when heap has not been intialized

char * pointer_to_hex_le(void *ptr);
void * malloc(size_t size);

int main(int argc, char * argv[]){

    malloc(9);

    return 0;
}

/*
* struct for allocation information for each chunk of data in heap
*/
struct alloc_info
{
    size_t size;
    struct alloc_info *next_info;
    struct alloc_info *prev_info;
};

/*
* Malloc function: allocate size bytes and return a pointer to the allocated memory (not intialized)
*/
void * malloc(size_t size)
{
    //round size to be divisible by 16
    size_t aligned_size; 
    char * sbrk_val, string1;

    string1 = "wohooo";

    aligned_size = (size/16 + 1)*16;

    //Case 0: heap has not been intialized (begin_ptr == NULL)
    if(begin_ptr == NULL)
    {
        begin_ptr = sbrk(0);

        sbrk_val = pointer_to_hex_le(begin_ptr);
        write(1, sbrk_val, 16);
        write(1,"\n",sizeof("\n"));

        sbrk_val = sbrk(INCREMENT_BY);

        sbrk_val = pointer_to_hex_le(sbrk(0));
        write(1, sbrk_val, 16);

        /*
        *begin_ptr = 12;
        *(begin_ptr+1) = 14;

        struct foo *fp = (struct food *) begin_ptr;
        */
    }

    //Case 1: heap has been initialized

    //iterate through every struct

    //Case 1a: there is size in our heap to allocate the chunk (between or at end)

    //Case 1b: there is no space in our heap (between or at end) sbrk here

    return begin_ptr;

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

char * pointer_to_hex_le(void *ptr)
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