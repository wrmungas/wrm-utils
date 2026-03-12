#ifndef WRM_MEMORY_H
#define WRM_MEMORY_H
/* --- HEADER DESCRIPTION -----------------------------------------------------
File memory.h

Created  Nov 14, 2025 
by William R Mungas (wrm)

(Last modified Nov 14, 2025)

DESCRIPTION:
Basic memory management schemes for applications. Each type of allocator takes 
an initial block of memory; the user decides whether this should be allowed to 
grow or not.

PROVIDES:
- pool type: a collection of objects of a known size, to be randomly accessed,
    modified, or removed
- stack type: a list of elements of known size with stack behavior, growing 
    until a rollback
- aligned_stack:

REQUIREMENTS:
Must link with C standard library

-----------------------------------------------------------------------------*/

#include "wrm/common.h"

/*--- Compile-time Constants ------------------------------------------------*/

#define MEM_GROWTH_FACTOR 2
#define MEM_POOL_MAX_CAPACITY UINT32_MAX


/*--- Type declarations -----------------------------------------------------*/

// represents a safe reference to an object in memory
typedef struct mem_Ref
mem_Ref;
// represents a pool allocator
typedef struct mem_Pool 
mem_Pool;
// represents a stack allocator: continually grows, only cleared on a call
// to reset()
typedef struct mem_Stack 
mem_Stack; 

typedef struct mem_Hexdump_Args
mem_Hexdump_Args;

/*--- Type definitions ------------------------------------------------------*/

struct mem_Ref {
    u32 idx;
    u32 src;
};

wrm_OPTION(mem_Ref, Ref);

struct mem_Pool {
    void *data; // pointer to memory of `item_cap` x `item_size` bytes
    size_t item_cap; // max number of items the pool can hold
    size_t item_size; // size in bytes of each item

    u32 id;
    u32 free_head; // start of the free list
    u32 free_cnt; // number of free slots available

    bool final; // whether the block of memory can be reallocated
};

struct mem_Stack {
    void *data; // pointer to memory of `cap` bytes
    size_t cap; // maximum number of bytes available
    size_t top; // index of the byte one past the top of the stack

    bool final; // whether the block of memory can be reallocated
};

struct mem_Hexdump_Args {
    void *data; // block of memory to print

    struct {
        struct {
            char *pre; // sequence to print before the bytes
            char *delim; // delimiter to print between each byte
            char *post; // sequence to print after the bytes
        } hex; // formatting for hex lines

        struct {
            char *pre; // sequence to print before the chars
            char *delim; // delimiter to print between each ascii char
            char *post; // sequence to print after the chars
        } ascii;
    } fmt;

    u32 bytes; // how many bytes to print, in total
    u32 bytes_per_line; // how many bytes to print on each line

    bool ascii; // whether or not to include ascii lines below each hex line
};


/*--- Function declarations -------------------------------------------------*/

#define mem_null (mem_Ref){.src = 0, .idx = 0}

/*
Prints the hexadecimal characters of a block of memory
Options controlled through `args`
TODO: incorporate this into log
*/
void mem_hexdump(mem_Hexdump_Args *args);

/*
Initialize the memory module with an internal resize() function
Must match the signature of the standard `realloc`
If NULL, no reallocation is allowed even if allocators have non-final memory
*/
void mem_init(wrm_FUNC(resize, void*, void*, size_t));

//--- Pool ---

/*
Initialize a pool memory allocator with room for `item_cap` elements, each 
`item_size` bytes.
If final, the memory will not be automatically resized internally and 
allocations may fail
*/
bool mem_init_pool(
    mem_Pool *p, 
    void *buf, 
    size_t isize, 
    size_t icap,
    bool final
);

/*
Get an open 'bucket' in the pool
If there are no slots and the pool cannot grow, returns a null Ref
*/
mem_Ref mem_palloc(mem_Pool *p);

/*
Get the slot in a pool that a given Ref points to
If the Ref is null or invalid (i.e. belonging to a different memory block),
returns NULL
*/
void *mem_pderef(mem_Pool *p, mem_Ref r);

/*
Free a slot in a pool, and alters the Ref to be null
Calls the provided delete() on the freed slot
Does nothing with a null Ref
*/
void mem_pfree(mem_Pool *p, mem_Ref *r);

/*
Calls the given `delete_item` function on all elements within the pool, if it
is not NULL.
Then calls the given `release` function on the overall memory block, if it is
not NULL.
Marks the pool as unusable
*/
void mem_delete_pool(
    mem_Pool *p, 
    wrm_FUNC(delete_item, void, void*), 
    wrm_FUNC(release, void, void*)
);

//--- Stack ---

/*
Initialize a stack memory allocator with a memory block of `size` total bytes
If final, the block will not be resized internally and allocations may fail
*/
bool mem_init_stack(mem_Stack *s, void *buf, size_t size, bool final);

/*
Allocate memory in a stack scheme
Grows the used part of the stack; if there is not enough room for the requested
number of bytes, returns a null Ref
*/
mem_Ref mem_stalloc(mem_Stack *s, size_t bytes);

/*
Allocate memory in a stack scheme, aligned to the next n-byte boundary
Grows the used part of the stack; if there is not enough room for the requested
number of bytes (plus any padded bytes to meet the requested alignment), 
returns a null Ref
*/
mem_Ref mem_stalloc_aligned(mem_Stack *s, size_t bytes, size_t n);

/*
Get the memory in a stack block that a Ref points to
If the Ref is null or invalid (i.e. pointing to a different memory block),
returns NULL
*/
void *mem_stderef(mem_Stack *s, mem_Ref r);

/*
Reset a stack (clearing the size to zero)
All previous Refs into the memory block are considered invalid
*/
void *mem_streset(mem_Stack *s);

/*
Calls the given free function on its memory block (if non-null)
Marks the stack as unusable
*/
void mem_delete_stack(mem_Stack *s, wrm_FUNC(free, void, void*));

#endif