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

#define INCREMENT_BY 160
static void *HEAP_BEGIN_PTR = NULL; // starts as NULL to know when heap has not been intialized
static struct alloc_info *FIRST_STRUCT = NULL; 

void *malloc(size_t size);
struct alloc_info *create_struct(void *starting_address, size_t size, struct alloc_info *next_info, struct alloc_info *prev_info); 
void increment_heap(size_t aligned_size, int space_left); 
int align16(int size);
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

    void *address; 

    // TEST 1: Unitialized heap (case 0) and requested size within the size of heap 
    // TEST 1: Unitilaized heap (case 0) and requested size outside size of heap (need to increment again) (change to 150)
    address = malloc(150); // address is 32 after starter pointer 
    write(1,"address of malloc(9):  ",sizeof("address of malloc(9):  "));
    write(1, pointer_to_hex_le(address), 16); 
    write(1,"\n\n",sizeof("\n\n"));

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
* Malloc function: allocate size bytes and return a pointer to the allocated memory (not intialized)
*/
void * malloc(size_t size)
{
    size_t aligned_request_size; 
    int struct_size_aligned;
    // SHOULD THERE BE A MAX / HOW DO YOU KNOW WHEN YOU'VE REACHED MAX? 

    // Relevant info to the loop
    struct alloc_info *next_alloc_ptr, *curr_alloc_ptr, *new_alloc_ptr; 
    size_t space_available; 
    size_t curr_align_size; 

    aligned_request_size = align16(size);
    struct_size_aligned = align16(sizeof(struct alloc_info));

    // Case 0: Heap has not been intialized
    if(HEAP_BEGIN_PTR == NULL)
    {
        write(1,"INITIALIZING HEAP\n",sizeof("INITIALIZING HEAP\n"));

        HEAP_BEGIN_PTR = sbrk(0);
         
        increment_heap(aligned_request_size, 0); 

        FIRST_STRUCT = create_struct(HEAP_BEGIN_PTR, size, NULL, NULL); 

        write(1,"HEAP_BEGIN_PTR: ",sizeof("HEAP_BEGIN_PTR: "));
        write(1, pointer_to_hex_le(FIRST_STRUCT), 16); 
        write(1,"\n",sizeof("\n"));

        return (char*)(FIRST_STRUCT) + struct_size_aligned; 
    }

    //Case 1: Heap has been initialized
    else
    {
        // what if the person frees the HEAP_BEGIN_PTR 
        curr_alloc_ptr = FIRST_STRUCT; 
        next_alloc_ptr = FIRST_STRUCT->next_info; 
        
        //iterate through every struct
        while(next_alloc_ptr != NULL)
        {
            curr_align_size = align16(curr_alloc_ptr->size);
            space_available = (next_alloc_ptr - curr_alloc_ptr + struct_size_aligned - curr_align_size);
            
             //Case 1a: there is size in our heap to allocate the chunk between
            if(space_available > (aligned_request_size + struct_size_aligned))
            {
                // Place struct in between existing allocations
                new_alloc_ptr = create_struct(curr_alloc_ptr + struct_size_aligned + curr_align_size, size, curr_alloc_ptr, next_alloc_ptr); 

                // Update current and next alloc info
                curr_alloc_ptr->next_info = new_alloc_ptr; 
                next_alloc_ptr->prev_info = new_alloc_ptr; 

                return (char*)(new_alloc_ptr) + struct_size_aligned; // address of allocation 
            }
            curr_alloc_ptr = next_alloc_ptr;
            next_alloc_ptr = curr_alloc_ptr->next_info; 
        }

       // write(1,"CASE 1B\n",sizeof("CASE 1B\n"));
        //Case 1b: Must add struct onto end of heap
        curr_align_size = align16(curr_alloc_ptr->size);
        
        space_available = ((struct alloc_info*)sbrk(0) - curr_alloc_ptr) - struct_size_aligned - curr_align_size;
        
        // write(1,"sbrk: ",sizeof("sbrk: "));
        // write(1, pointer_to_hex_le((struct alloc_info*)sbrk(0)), 16); 
        // write(1,"\n",sizeof("\n"));

        // write(1,"curr alloc ptr: ",sizeof("curr alloc ptr: "));
        // write(1, pointer_to_hex_le(curr_alloc_ptr), 16); 
        // write(1,"\n",sizeof("\n"));

        // write(1,uint64_to_string(((struct alloc_info*)sbrk(0) - curr_alloc_ptr)), 16);
        // write(1," <- diff betwen sbrk and curr alloc ptr\n",sizeof("<- diff betwen sbrk and curr alloc ptr\n"));

        // write(1,uint64_to_string((aligned_request_size)), 16);
        // write(1,"\n",sizeof("\n"));
        // write(1,uint64_to_string((struct_size_aligned)), 16);
        // write(1,"\n",sizeof("\n"));
        // write(1,uint64_to_string((aligned_request_size + struct_size_aligned)), 16);
        // write(1,"\n",sizeof("\n"));

        // Check if heap does not have enough space at the end 
        if(space_available < (aligned_request_size + struct_size_aligned))
        {
            increment_heap(aligned_request_size, space_available); //print statement in this function
        }

        // Place struct at end of heap 
        new_alloc_ptr = create_struct((char*)(curr_alloc_ptr) + struct_size_aligned + curr_align_size, size, curr_alloc_ptr, next_alloc_ptr); 
        
        // Update current pointer info
        curr_alloc_ptr->next_info = new_alloc_ptr;

    }

    return (char*)(new_alloc_ptr) + struct_size_aligned; // address of allocation

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
    write(1,"INCREMENTING HEAP\n",sizeof("INCREMENTING HEAP\n"));

    sbrk(INCREMENT_BY); // set this to sbrk_val to see what is stored there
    space_left += INCREMENT_BY;

    // If size is greater than INCREMENT_BY, grow heap as necessary 
    while(aligned_size + align16(sizeof(struct alloc_info)) > space_left)
    {
        write(1,"INCREMENTING MORE\n",sizeof("INCREMENTING MORE\n"));

        sbrk(INCREMENT_BY);
        space_left += INCREMENT_BY;
    }
}

/*
* Rounds int to nearest larger power of 16
*/
int align16(int size)
{
    int align_by;
    align_by = 16;

    return (size/align_by + 1) * align_by; //round size to be divisible by 16
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
