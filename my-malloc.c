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
#include <limits.h>

#define INCREMENT_BY 160
static void *heap_begin_ptr = NULL;
static void *heap_end_ptr = NULL;
static struct alloc_info *first_struct = NULL;

void *malloc(size_t size);
struct alloc_info *create_alloc_info(void *starting_address, size_t size, struct alloc_info *prev_info, struct alloc_info *next_info);
void *increment_heap(size_t aligned_size, size_t space_left);
void free(void *ptr);
void *calloc(size_t nmemb, size_t size);
void *realloc(void *ptr, size_t size);
size_t malloc_usable_size(void *ptr);
size_t align16(size_t size);
size_t get_space_available(struct alloc_info *left_ptr, struct alloc_info *right_ptr, size_t alloc_size);
char *pointer_to_hex_le(void *ptr); // delete
char *uint64_to_string(uint64_t n);

/*
* Struct for allocation information for each chunk of data in heap
*/
struct alloc_info
{
    size_t size;
    struct alloc_info *prev_info;
    struct alloc_info *next_info;
};

/*
* Allocate size bytes and return a pointer to the allocated memory (not intialized)
* On error, return NULL. 
* NULL may also be returned by a successful call to malloc() with a size of zero.
*/
void * malloc(size_t size)
{
    size_t aligned_request_size;
    size_t struct_size_aligned;

    // Info relevant to the loop
    struct alloc_info *next_alloc_ptr, *curr_alloc_ptr, *new_alloc_ptr; 
    size_t space_available; 
    size_t curr_align_size; 

    aligned_request_size = align16(size);
    struct_size_aligned = align16(sizeof(struct alloc_info));
    
    // Check if malloc is called with size = 0
    if(size == 0)
    {
        return NULL;
    }

    // Case 0: Heap has not been intialized
    if(first_struct == NULL)
    { 
        if(heap_begin_ptr == NULL)
        {
            // Initialize heap_begin_ptr and handle errors
            if((heap_begin_ptr = sbrk(0)) == (void *)-1)
            {
                return NULL;
            }

            if (increment_heap(aligned_request_size, 0) == NULL)
            {
                return NULL;
            }
        }

        first_struct = create_alloc_info(heap_begin_ptr, size, NULL, NULL); 
        write(1, "first struct initialized\n", sizeof("first struct initialized\n"));
        return (void*)((char*)(first_struct) + struct_size_aligned); 
    }

    // Case 1: Heap has been initialized
    // Case 1a: If there is space at the beginning of the heap, place allocation there
    if(((char*)first_struct - (char*)heap_begin_ptr) > (aligned_request_size + struct_size_aligned))
    {
        first_struct = create_alloc_info(heap_begin_ptr, size, NULL, first_struct);
        write(1, "space at beginning\n", sizeof("space at beginning\n"));
        return (void*)((char*)(first_struct) + struct_size_aligned);
    }

    curr_alloc_ptr = first_struct; 
    next_alloc_ptr = first_struct->next_info; 
    
    //Case 1b: Check if allocation can be placed in between previous allocations
    while(next_alloc_ptr != NULL)
    {
        curr_align_size = align16(curr_alloc_ptr->size);
        space_available = get_space_available(next_alloc_ptr, curr_alloc_ptr, curr_alloc_ptr->size);

        //Case 1ba: there is size in our heap to allocate the chunk between
        if(space_available >= (aligned_request_size + struct_size_aligned))
        {
            // Place struct in between existing allocations
            new_alloc_ptr = create_alloc_info((char*)curr_alloc_ptr + struct_size_aligned + curr_align_size, size, curr_alloc_ptr, next_alloc_ptr); 
            write(1, "space in between structs\n", sizeof("space in between structs\n"));
            // address of allocation
            return (void*)((char*)(new_alloc_ptr) + struct_size_aligned);  
        }

        // Increment for the loop
        curr_alloc_ptr = next_alloc_ptr;
        next_alloc_ptr = next_alloc_ptr->next_info; 
    }
    
    //Case 1bb: Must add struct onto end of heap
    curr_align_size = align16(curr_alloc_ptr->size);
    space_available = get_space_available(heap_end_ptr, curr_alloc_ptr, curr_alloc_ptr->size);        

    // Check if heap does not have enough space at the end 
    if(space_available < (aligned_request_size + struct_size_aligned))
    {
        if(increment_heap(aligned_request_size, space_available) == NULL)
        {
            return NULL;
        }
    }

    // Place struct at end of heap 
    new_alloc_ptr = create_alloc_info((char*)(curr_alloc_ptr) + struct_size_aligned + curr_align_size, size, curr_alloc_ptr, next_alloc_ptr); 
    write(1, "placed at end\n", sizeof("placed at end\n"));
    // Address of allocation
    return (void*)((char*)(new_alloc_ptr) + struct_size_aligned); 
}

/*
* Create struct alloc_info with memory data allocation in the heap. 
*/
struct alloc_info *create_alloc_info(void *starting_address, size_t size, struct alloc_info *prev_info, struct alloc_info *next_info)
{
    struct alloc_info *metadata;

    metadata = starting_address;

    metadata->size = size;
    metadata->prev_info = prev_info;
    metadata->next_info = next_info;

    // Update pointer of prev_info->next_info if provided
    if(prev_info != NULL)
    {
        prev_info->next_info = metadata;
    }

    // Update pointer of next_info->prev_info if provided
    if(next_info != NULL)
    {
        next_info->prev_info = metadata;
    }

    return metadata; 
}

/*
* Increment the heap (move the break) until there is 
* enough space to store a requested allocation of aligned_size. 
*/
void *increment_heap(size_t aligned_size, size_t space_left)
{

    // If align size requested and struct size is greater than INCREMENT_BY, grow heap as necessary 
    while(aligned_size + align16(sizeof(struct alloc_info)) > space_left)
    {
        if(sbrk(INCREMENT_BY) == (void *)-1)
        {
            return NULL;
        }

        space_left += INCREMENT_BY;
    }

    // Update heap_end_ptr and handle errors
    if((heap_end_ptr = sbrk(0)) == (void *)-1)
    {
        return NULL;
    }

    return heap_end_ptr;
}

/*
* Free the memory space pointed to by ptr, a pointer returned by a previous 
* malloc(),  calloc(), or  realloc(). If ptr is NULL, no operation is
* performed.
*/
void free(void *ptr)
{
    struct alloc_info *prev_alloc_ptr, *curr_alloc_ptr, *next_alloc_ptr; 
    
    if(ptr != NULL)
    {
        // Move ptr to beginning of struct
        curr_alloc_ptr = (struct alloc_info *)((char*)ptr - align16(sizeof(struct alloc_info)));

        next_alloc_ptr = curr_alloc_ptr->next_info;
        prev_alloc_ptr = curr_alloc_ptr->prev_info;

        // Case 0: curr is between two allocations
        if(next_alloc_ptr != NULL && prev_alloc_ptr != NULL)
        {
            prev_alloc_ptr->next_info = next_alloc_ptr;
            next_alloc_ptr->prev_info = prev_alloc_ptr;
        }
        // Case 1: curr is at end of allocations
        else if(next_alloc_ptr == NULL && prev_alloc_ptr != NULL)
        {
            prev_alloc_ptr->next_info = NULL;
        }
        // Case 2: curr is first allocation in heap
        else if(next_alloc_ptr != NULL && prev_alloc_ptr == NULL)
        {
            first_struct = next_alloc_ptr;
            next_alloc_ptr->prev_info = NULL;
        }
        // Case 3: curr is only allocation in heap
        else
        {
            first_struct = NULL;
        }
    }
}

/*
* Allocate memory for an array of nmemb elements of size bytes each and return
* a pointer to the allocated memory, which is set to zero. 
* If nmemb or size is 0, then return NULL. 
* If the multiplication of nmemb and size would result
* in integer  overflow, then calloc() returns an error (NULL).
*/
void *calloc(size_t nmemb, size_t size)
{
    void *malloc_address;
    size_t space_needed;

    if(nmemb == 0 || size == 0)
    {
        return NULL; 
    }

    // Check for size_t overflow 
    if(nmemb > (SSIZE_MAX / size))
    {
       return NULL;
    }

    space_needed = nmemb*size;

    if((malloc_address = malloc(space_needed)) == NULL)
    {
        return NULL;
    }

    memset(malloc_address, 0, space_needed);
    return malloc_address;
}

/*
* Change the size of the memory block pointed to by ptr to size bytes.
*/
void *realloc(void *ptr, size_t size)
{
    void *new_ptr;
    struct alloc_info *og_alloc_ptr, *og_next_alloc_ptr;
    size_t space_available;

    // Case 0: When ptr is NULL, act like malloc
    if(ptr == NULL)
    {
        if((new_ptr = malloc(size)) == NULL)
        {
            return NULL;
        }

        return new_ptr;
    }
    
    
    // Case 1: When ptr is not NULL and size is 0, act like free 
    if(size == 0)
    {
        free(ptr);
        return NULL; 
    }

    // Case 2: ptr not NULL and size is not 0
    og_alloc_ptr = (struct alloc_info *)((char*)ptr - align16(sizeof(struct alloc_info)));
    og_next_alloc_ptr = og_alloc_ptr->next_info;
    
    // Case 2a: Reallocing on the last allocation in the heap
    if(og_next_alloc_ptr == NULL)
    {
        free(ptr);
        if((new_ptr = malloc(size)) == NULL)
        {
            return NULL;
        }
        return new_ptr;
    }

    // Case 2b: Reallocing on allocations in between other allocations 
    space_available = get_space_available(og_next_alloc_ptr, og_alloc_ptr, 0); 

    // Case 2ba: Room to expand allocation in place
    if(space_available >= align16(size))
    {
        og_alloc_ptr->size = size; 
        return ptr; 
    }

    //Case 2bb: Not enough space, realloc at the end of the heap
    free(ptr);
    if((new_ptr = malloc(size)) == NULL)
    {
        return NULL;
    }

    return new_ptr;
}

/*
* Returns  the  number  of  usable bytes  in the block pointed to by ptr. 
*/
size_t malloc_usable_size(void *ptr)
{
    struct alloc_info *curr_alloc_ptr; 

    curr_alloc_ptr = (struct alloc_info *)((char*)ptr - align16(sizeof(struct alloc_info)));
    
    return (curr_alloc_ptr->size); 
}

/*
* Rounds size_t to nearest larger power of 16
*/
size_t align16(size_t size)
{
    size_t align_by, align_val;
    align_by = 16;

    align_val = (size/align_by + 1) * align_by;

    if(size == 0)
    {
        align_val = 0;
    }

    return align_val;
}

/*
* Return the number of unused bytes in between two allocations in the heap
*/
size_t get_space_available(struct alloc_info *next_ptr, struct alloc_info *curr_ptr, size_t alloc_size)
{
    size_t struct_size_aligned, alloc_size_aligned, space_available;

    struct_size_aligned = align16(sizeof(struct alloc_info));
    alloc_size_aligned = align16(alloc_size);

    space_available = ((char*)next_ptr - (char*)curr_ptr) - struct_size_aligned - alloc_size_aligned;
    
    return space_available;
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
