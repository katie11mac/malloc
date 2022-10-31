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
#define ALIGN_BY 16
static void *HEAP_BEGIN_PTR = NULL; // starts as NULL to know when heap has not been intialized
static struct alloc_info *FIRST_STRUCT = NULL; 

void *malloc(size_t size);
struct alloc_info *create_struct(void *starting_address, size_t size, struct alloc_info *next_info, struct alloc_info *prev_info); 
void increment_heap(size_t aligned_size, int space_left); 
char *pointer_to_hex_le(void *ptr); // delete
char *uint64_to_string(uint64_t n);

/*
* struct for allocation information for each chunk of data in heap
*/
struct alloc_info
{
    size_t size;
    struct alloc_info *prev_info;
    struct alloc_info *next_info;
};

int main(int argc, char *argv[]){

    malloc(9);

    return 0;
}

/*
* Malloc function: allocate size bytes and return a pointer to the allocated memory (not intialized)
*/
void * malloc(size_t size)
{
    size_t aligned_request_size; 
    struct alloc_info *metadata;
    char *sbrk_val;
    int space_left; // SHOULD THERE BE A MAX / HOW DO YOU KNOW WHEN YOU'VE REACHED MAX? 

    // Relevant info to the loop
    struct alloc_info *next_alloc_ptr, *curr_alloc_ptr; 
    int space_available; 
    size_t curr_align_size; 

    aligned_request_size = (size/ALIGN_BY + 1) * ALIGN_BY; //round size to be divisible by 16

    // Case 0: Heap has not been intialized
    if(HEAP_BEGIN_PTR == NULL)
    {
        HEAP_BEGIN_PTR = sbrk(0);
            
        increment_heap(aligned_request_size, 0); 

        metadata = create_struct(HEAP_BEGIN_PTR, size, NULL, NULL); 
        FIRST_STRUCT = metadata; 
        return FIRST_STRUCT + STRUCT_SIZE; 
    }

    //Case 1: Heap has been initialized
    else
    {
        //metadata may not be initialized, so maybe start at the first allocation, which is HEAP_BEGIN_PTR
        // what if the person frees the HEAP_BEGIN_PTR 
        curr_alloc_ptr = FIRST_STRUCT; 
        next_alloc_ptr = FIRST_STRUCT->next_info; 
        
        //iterate through every struct
        while(next_alloc_ptr != NULL)
        {
            curr_align_size = (curr_alloc_ptr->size/ALIGN_BY + 1) * ALIGN_BY;
            space_available = ((next_alloc_ptr - curr_alloc_ptr) - STRUCT_SIZE) - curr_align_size;
            
             //Case 1a: there is size in our heap to allocate the chunk between
            if(space_available > (aligned_request_size + STRUCT_SIZE))
            {
                //put in between, return pointer to beginning of memory (results of create_struct + STRUCT_SIZE)
            }
            // update pointers 
        }
        //Case 1b: we must add struct onto end of heap

        // add the struc to the end 
            // check if you increment: increment heap 
        // create the struct 

    }

    return HEAP_BEGIN_PTR;

}

/*
* Create struct alloc_info with memory data allocation in the heap. 
*/
struct alloc_info *create_struct(void *starting_address, size_t size, struct alloc_info *prev_info, struct alloc_info *next_info)
{
    struct alloc_info *metadata;

    metadata = starting_address;

    metadata->size = size;
    metadata->prev_info = prev_info;
    metadata->next_info = next_info;

    // struct alloc_info *metadata = (struct alloc_info *) starting_address; 
    // write(1, uint64_to_string(metadata->size), 8); 
    // write(1,"\n",sizeof("\n"));
    //pointer_math = ((struct alloc_info**)(starting_address)) + 1;
    // write(1, pointer_to_hex_le(metadata->prev_info), 16);
    // write(1,"\n",sizeof("\n"));
    // write(1, pointer_to_hex_le(metadata->next_info), 16);
    // write(1,"\n",sizeof("\n"));
    return metadata; 
}

void increment_heap(size_t aligned_size, int space_left)
{
    sbrk(INCREMENT_BY); // set this to sbrk_val to see what is stored there
    space_left += INCREMENT_BY;

    // If size is greater than INCREMENT_BY, grow heap as necessary 
    while(aligned_size + STRUCT_SIZE > space_left)
    {
        sbrk(INCREMENT_BY);
        space_left += INCREMENT_BY;
    }
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
