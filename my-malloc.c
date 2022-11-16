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

#define INCREMENT_BY 800
static void *heap_begin_ptr = NULL; // starts as NULL to know when heap has not been intialized
static void *heap_end_ptr = NULL;
static struct alloc_info *first_struct = NULL;

void *malloc(size_t size);
struct alloc_info *create_alloc_info(void *starting_address, size_t size, struct alloc_info *next_info, struct alloc_info *prev_info);
void increment_heap(size_t aligned_size, size_t space_left);
void free(void *ptr);
void *calloc(size_t nmemb, size_t size);
void *realloc(void *ptr, size_t size);
size_t malloc_usable_size(void *ptr);
size_t align16(size_t size);
size_t get_space_available(struct alloc_info *left_ptr, struct alloc_info *right_ptr, size_t alloc_size);
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

/*
* Allocate size bytes and return a pointer to the allocated memory (not intialized)
* On error, return NULL. 
* NULL may also be returned by a successful call  to  malloc() with a size of zero.
*/
void * malloc(size_t size)
{
    // write(1,"MALLOC\n",sizeof("MALLOC\n"));
    size_t aligned_request_size; 
    size_t struct_size_aligned;

    // Relevant info to the loop
    struct alloc_info *next_alloc_ptr, *curr_alloc_ptr, *new_alloc_ptr; 
    size_t space_available; 
    size_t curr_align_size; 

    aligned_request_size = align16(size);
    struct_size_aligned = align16(sizeof(struct alloc_info));

    // Case 0: Heap has not been intialized
    if(first_struct == NULL)
    { 
        if(heap_begin_ptr == NULL)
        {
            //write(1,"INITIALIZING HEAP\n",sizeof("INITIALIZING HEAP\n"));

            // Initialize heap_begin_ptr and handle errors
            if((heap_begin_ptr = sbrk(0)) == (void *)-1)
            {
                perror("sbrk");
                exit(1);
            }
            
            increment_heap(aligned_request_size, 0); 
        }

        first_struct = create_alloc_info(heap_begin_ptr, size, NULL, NULL); 

        // write(1,"heap_begin_ptr: ",sizeof("heap_begin_ptr: "));
        // write(1, pointer_to_hex_le(first_struct), 16); 
        // write(1,"\n",sizeof("\n"));

        return (void*)((char*)(first_struct) + struct_size_aligned); 
        //we can't do +1 because size of struct is not 16 aligned
    }

    //Case 1: Heap has been initialized
    else
    {

        // Case 1a: If there is space at the beginning of the heap, place allocation there
        if(((char*)first_struct - (char*)heap_begin_ptr) > (aligned_request_size + struct_size_aligned))
        {
            first_struct = create_alloc_info(heap_begin_ptr, size, NULL, first_struct);

            // Update the next struct's previous info
            (((struct alloc_info*)first_struct)->next_info)->prev_info = first_struct; 
            return (void*)((char*)(first_struct) + struct_size_aligned);
        }

        curr_alloc_ptr = first_struct; 
        next_alloc_ptr = first_struct->next_info; 
        
        //Case 1b: Check if allocation can be placed in between previous allocations
        while(next_alloc_ptr != NULL)
        {
            curr_align_size = align16(curr_alloc_ptr->size);
            
            // SHOULD PROBABLY CHECK LINE BELOW TOO
            space_available = get_space_available(next_alloc_ptr, curr_alloc_ptr, curr_alloc_ptr->size);

             //Case 1a: there is size in our heap to allocate the chunk between
            if(space_available >= (aligned_request_size + struct_size_aligned))
            {
                // Place struct in between existing allocations
                new_alloc_ptr = create_alloc_info((char*)curr_alloc_ptr + struct_size_aligned + curr_align_size, size, curr_alloc_ptr, next_alloc_ptr); 

                // Update current and next alloc info
                ((struct alloc_info*)curr_alloc_ptr)->next_info = new_alloc_ptr; 
                ((struct alloc_info*)next_alloc_ptr)->prev_info = new_alloc_ptr; 

                return (void*)((char*)(new_alloc_ptr) + struct_size_aligned); // address of allocation 
            }

            // Increment for the loop
            curr_alloc_ptr = next_alloc_ptr;
            next_alloc_ptr = next_alloc_ptr->next_info; 
        }
       
        //Case 1c: Must add struct onto end of heap
        curr_align_size = align16(curr_alloc_ptr->size);
        
        space_available = get_space_available(heap_end_ptr, curr_alloc_ptr, curr_alloc_ptr->size);        
        // write(1,"sbrk: ",sizeof("sbrk: "));
        // write(1, pointer_to_hex_le(heap_end_ptr), 16); 
        // write(1,"\n",sizeof("\n"));

        // write(1,"curr alloc ptr: ",sizeof("curr alloc ptr: "));
        // write(1, pointer_to_hex_le(curr_alloc_ptr), 16); 
        // write(1,"\n",sizeof("\n"));

        // write(1,uint64_to_string((char*)heap_end_ptr - (char*)curr_alloc_ptr), 16);
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
        new_alloc_ptr = create_alloc_info((char*)(curr_alloc_ptr) + struct_size_aligned + curr_align_size, size, curr_alloc_ptr, next_alloc_ptr); 
        
        // Update current pointer info
        ((struct alloc_info*)curr_alloc_ptr)->next_info = new_alloc_ptr;

    }

    return (void*)((char*)(new_alloc_ptr) + struct_size_aligned); // address of allocation
}

/*
* Create struct alloc_info with memory data allocation in the heap. 
*/
struct alloc_info *create_alloc_info(void *starting_address, size_t size, struct alloc_info *prev_info, struct alloc_info *next_info)
{
    // write(1,"CREATE STRUCT\n",sizeof("CREATE STRUCT\n"));
    struct alloc_info *metadata;

    metadata = starting_address;

    metadata->size = size;
    metadata->prev_info = prev_info;
    metadata->next_info = next_info;

    if(prev_info != NULL)
    {
        prev_info->next_info = metadata;
    }

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

/*
* Increment the heap, in other words move the break, until there is 
* enough space to store a requested allocation of aligned_size. 
*/
void increment_heap(size_t aligned_size, size_t space_left)
{
    // write(1,"INCREMENT_HEAP\n",sizeof("INCREMENT_HEAP\n"));

    // set this to sbrk_val to see what is stored there
    if(sbrk(INCREMENT_BY) == (void *)-1)
    {
        perror("sbrk"); 
        exit(1); 
    }

    space_left += INCREMENT_BY;

    // If size is greater than INCREMENT_BY, grow heap as necessary 
    while(aligned_size + align16(sizeof(struct alloc_info)) > space_left)
    {
        // write(1,"INCREMENTING MORE\n",sizeof("INCREMENTING MORE\n"));

        if(sbrk(INCREMENT_BY) == (void *)-1)
        {
            perror("sbrk"); 
            exit(1); 
        }

        space_left += INCREMENT_BY;
    }

    // Update heap_end_ptr and handle errors
    if((heap_end_ptr = sbrk(0)) == (void *)-1)
    {
        perror("sbrk"); 
        exit(1); 
    }

    // write(1,"heap_end_ptr: ",sizeof("heap_end_ptr: "));
    // write(1, pointer_to_hex_le(heap_end_ptr), 16); 
    // write(1,"\n",sizeof("\n"));
    
}

/*
* Free the memory space pointed to by ptr, a pointer returned by a previous 
* malloc(),  calloc(), or  realloc(). If ptr is NULL, no operation is
* performed.
*/
void free(void *ptr)
{
    // write(1,"FREE\n",sizeof("FREE\n"));
    struct alloc_info *prev_alloc_ptr, *curr_alloc_ptr, *next_alloc_ptr; 
    
    if(ptr != NULL)
    {
        //move ptr to beginning of struct
        curr_alloc_ptr = (struct alloc_info *)((char*)ptr - align16(sizeof(struct alloc_info)));
        // write(1, pointer_to_hex_le(curr_alloc_ptr), 16); 
        // write(1, pointer_to_hex_le(ptr), 16); 

        next_alloc_ptr = curr_alloc_ptr->next_info;
        prev_alloc_ptr = curr_alloc_ptr->prev_info;

        // write(1,"next_alloc_ptr:  ",sizeof("next_alloc_ptr:  "));
        // //write(1, pointer_to_hex_le(next_alloc_ptr), 16); 
        // write(1,"\n",sizeof("\n"));

        // write(1,"prev_alloc_ptr:  ",sizeof("prev_alloc_ptr:  "));
        // write(1, pointer_to_hex_le(prev_alloc_ptr), 16); 
        // write(1,"\n",sizeof("\n"));

        //Case 0: curr is between two allocations
        if(next_alloc_ptr != NULL && prev_alloc_ptr != NULL)
        {
            // write(1,"running case 0\n",sizeof("running case 0\n"));
            prev_alloc_ptr->next_info = next_alloc_ptr;
            next_alloc_ptr->prev_info = prev_alloc_ptr;
        }
        //Case 1: curr is at end of allocations
        else if(next_alloc_ptr == NULL && prev_alloc_ptr != NULL)
        {
            prev_alloc_ptr->next_info = NULL;
        }
        //Case 2: curr is first allocation in heap
        else if(next_alloc_ptr != NULL && prev_alloc_ptr == NULL)
        {
            first_struct = next_alloc_ptr;
            next_alloc_ptr->prev_info = NULL;
        }
        //Case 3: curr is only allocation in heap
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
    // write(1,"CALLOC\n",sizeof("CALLOC\n"));
    void *malloc_address;
    size_t space_needed;

    if(nmemb == 0 || size == 0)
    {
        return NULL; 
    }

    // check for size_t overflow 
    if(nmemb > (SSIZE_MAX / size))
    {
       return NULL;
    }

    space_needed = nmemb*size;
    malloc_address = malloc(space_needed);
    memset(malloc_address, 0, space_needed);
    return malloc_address;

}

/*
* Change the size of the memory block pointed to by ptr to size bytes.
*/
void *realloc(void *ptr, size_t size)
{
    // write(1,"REALLOC\n",sizeof("REALLOC\n"));
    void *new_ptr;
    struct alloc_info *og_alloc_ptr, *og_next_alloc_ptr;
    size_t space_available;


    // Case 0: When ptr is NULL, act like malloc
    if(ptr == NULL)
    {
        new_ptr = malloc(size);
        return new_ptr;
    }
    
    // Case 1: When ptr is not NULL
    // else 
    // {

        // Case 1a: When size is 0, act like free 
        if(size == 0)
        {
            free(ptr);
            return NULL; 
        }
        // Size is not 0
        // else
        // {
            og_alloc_ptr = (struct alloc_info *)((char*)ptr - align16(sizeof(struct alloc_info)));
            og_next_alloc_ptr = og_alloc_ptr->next_info;
            
            // Trying to realloc at the end 
            if(og_next_alloc_ptr == NULL)
            {
                // write(1, "***reallocing at the end***\n", sizeof("***reallocing at the end***\n")); 
                free(ptr);
                new_ptr = malloc(size);
                return new_ptr;
            }

            space_available = get_space_available(og_next_alloc_ptr, og_alloc_ptr, 0); 

            // write(1,"\nspace aval for realloc: ",sizeof("\nspace aval for realloc: "));
            // write(1, uint64_to_string(space_available), 16); 
            // write(1,"\n\n",sizeof("\n\n"));
        
            // Case 1b: Room to expand allocation in place 
            // if need to debug double check this 
            if(space_available >= align16(size))
            {
                og_alloc_ptr->size = size; 
                return ptr; 
            }
            //Case 1c: Not enough space, realloc at the end of the heap
            // else
            // {
                free(ptr);
                new_ptr = malloc(size);
                return new_ptr;
            // }
        // }
    // }

    return NULL;

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
    return align_val; //round size to be divisible by 16
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
    
    // write(1, uint64_to_string(alloc_size_aligned), 16); 
    // write(1,"= alloc_size_aligned \n",sizeof("= alloc_size_aligned \n"));

    // write(1,"\n leftptr-rightptr=",sizeof("\n leftptr-rightptr=")); 
    // write(1, uint64_to_string((char*)left_ptr - (char*)right_ptr), 16); 

    // write(1,"\n space_available=",sizeof("\n space_available=")); 
    // write(1, uint64_to_string(space_available), 16); 
    // //write(1,"freeing our malloc(9)",sizeof("freeing our malloc(9)"));
    // write(1,"\n\n",sizeof("\n\n")); 

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
