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
#include <limits.h> // need for printing size_t

#define INCREMENT_BY 48
#define STRUCT_SIZE 32
static void * begin_ptr = NULL; // starts as NULL so know when heap has not been intialized

/*
* struct for allocation information for each chunk of data in heap
*/
struct alloc_info
{
    size_t size;
    struct alloc_info *prev_info;
    struct alloc_info *next_info;
};

char * pointer_to_hex_le(void *ptr);
char *uint64_to_string(uint64_t n);
void * malloc(size_t size);
struct alloc_info * create_struct(void * starting_address, size_t size, struct alloc_info *next_info, struct alloc_info *prev_info); 


int main(int argc, char * argv[]){

    malloc(9);

    return 0;
}

/*
* Malloc function: allocate size bytes and return a pointer to the allocated memory (not intialized)
*/
void * malloc(size_t size)
{
    //round size to be divisible by 16
    size_t aligned_size; 
    struct alloc_info * metadata; 
    char * sbrk_val;

    aligned_size = (size/16 + 1)*16;

    //Case 0: heap has not been intialized (begin_ptr == NULL)
    if(begin_ptr == NULL)
    {
        begin_ptr = sbrk(0);

        sbrk_val = pointer_to_hex_le(begin_ptr);
        write(1, sbrk_val, 16);
        write(1,"\n",sizeof("\n"));

        sbrk_val = sbrk(INCREMENT_BY); // set this to sbrk_val to see what is stored there
        
        sbrk_val = pointer_to_hex_le(sbrk(0));
        write(1, sbrk_val, 16);
        write(1,"\n",sizeof("\n"));

        metadata = create_struct(begin_ptr, size, NULL, NULL); 

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

struct alloc_info * create_struct(void * starting_address, size_t size, struct alloc_info *prev_info, struct alloc_info *next_info)
{
    
    struct alloc_info **pointer_math; 

    *((size_t*)starting_address) = size;

    write(1, pointer_to_hex_le(starting_address), 16);
    write(1, "= starting address \n starting address+8 =", sizeof("= starting address \n starting address+8 ="));
    pointer_math = ((size_t*)(starting_address)) + 1;
    write(1, pointer_to_hex_le(pointer_math), 16);
    write(1,"\n",sizeof("\n"));

    // *((struct alloc_info**) pointer_math) = next_info;

    


    // struct alloc_info *metadata = (struct alloc_info *) starting_address; 
    // write(1, uint64_to_string(metadata->size), 8); 
    write(1,"\n",sizeof("\n"));
    //pointer_math = ((struct alloc_info**)(starting_address)) + 1;
    // write(1, pointer_to_hex_le(metadata->prev_info), 16);
    // write(1, pointer_to_hex_le(metadata->next_info), 16);
    return metadata; 
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

char *
uint64_to_string(uint64_t n)
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
