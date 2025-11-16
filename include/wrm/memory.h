#ifndef WRM_MEMORY_H
#define WRM_MEMORY_H

/*
File memory.h

Created  Nov 14, 2025 
by William R Mungas (wrm)

Version: 0.1.0 
(Last modified Nov 14, 2025)

DESCRIPTION:
Basic memory structures for efficient applications

PROVIDES:
- pool type: a collection of objects of a known size, to be randomly accessed, modified, or removed
- stack type: a list of elements of known size with stack behavior, growing until a rollback

REQUIREMENTS:
Must link with C standard library

*/

#include "wrm/common.h"

#define WRM_MEMORY_GROWTH_FACTOR 2

#define WRM_POOL_MAX_CAPACITY UINT32_MAX

/*
Type declarations
*/

// represents a pool of elements
typedef struct wrm_Pool wrm_Pool;
// represents a continually-growing stack of elements
typedef struct wrm_Stack wrm_Stack;


/*
Type definitions
*/

struct wrm_Pool {
    size_t element_size;
    size_t cap;
    size_t used;
    void *data;
    bool *is_used;
    bool auto_reserve;
};

struct wrm_Stack {
    size_t element_size;
    size_t cap;
    size_t len;
    void *data;
    bool auto_reserve;
};


/*
Function declarations
*/

// pool

/* 
Initialize a pool of `capacity` elements of `element_size` size
Returns `true` if the operation was successful
`auto_reserve` means the pool will automatically allocate space for new elements 
*/
bool wrm_Pool_init(wrm_Pool *p, size_t capacity, size_t element_size, bool auto_reserve);
/* 
Ensure that pool `p` has room for `capacity` total elements by GROWING ONLY 
Returns `true` if the operation was successful
*/
bool wrm_Pool_reserve(wrm_Pool *p, size_t capacity);
/* 
If `capacity` is >= the number of elements currently in the bool, shrinks the pool's capacity to use less memory
Returns `true` if the operation was successful
Never automatically called
*/
bool wrm_Pool_shrink(wrm_Pool *p, size_t capacity);
/* Get an available slot (index of an element) from pool `p` */
wrm_Option_Handle wrm_Pool_getSlot(wrm_Pool *p);
/* Check that the slot at a given index is valid for pool `p` */
inline bool wrm_Pool_isValid(wrm_Pool *p, wrm_Handle idx)
{
    return idx < p->cap && p->is_used[idx];
}
/* Release the slot at `idx` for reuse, if it wasn't already available, in pool `p` */
inline void wrm_Pool_freeSlot(wrm_Pool *p, wrm_Handle idx)
{
    if(!wrm_Pool_isValid(p, idx)) { return; }
    p->is_used[idx] = false;
    p->used--;
}
/* Release the resources associated with pool `p`, which is no longer considered usable after this point */
void wrm_Pool_delete(wrm_Pool *p);

/* Type-generic pool accessor macros for `pool` (NOT a pointer) and type name `t` */ 

// `pool` must NOT be a pointer; guard any raw access with `wrm_Pool_isValid()`
#define wrm_Pool_AS(pool, t) ((t*)((pool).data))
// `pool` must NOT be a pointer; guard any raw access with `wrm_Pool_isValid()`; this gives a t* to the instance at the desired index
#define wrm_Pool_AT(pool, t, idx) (wrm_Pool_AS(pool, t) + idx)

// stack

/* 
Initialize a stack with room for `capacity` elements of size `element_size`
Returns `true` if the operation was successful
If `auto_reserve` is false you must manually allocate additional capacity with `reserve()` 
*/
bool wrm_Stack_init(wrm_Stack *s, size_t capacity, size_t element_size, bool auto_reserve);
/* 
Ensure that stack `s` has room for `capacity` total elements
Returns `true` if the operation was successful 
*/
bool wrm_Stack_reserve(wrm_Stack *s, size_t capacity);
/* 
If `capacity` is <= the stack's current capacity, shrinks the stack's capacity to use less memory
Returns `true` if the operation was successful
Never automatically called 
*/
bool wrm_Stack_shrink(wrm_Stack *s, size_t capacity);
/*
Grow's the stack's length by one and returns the index of the top element
Fails if the stack cannot grow
*/
wrm_Option_Handle wrm_Stack_push(wrm_Stack *s);
/* Roll back stack `s` to `len`; all elements beyond `len` are now considered invalid */
inline void wrm_Stack_reset(wrm_Stack *s, size_t len)
{
    if(len > s->len) return;
    s->len = len;
}
/* Release the resources associated with stack `s`, which is no longer considered usable after this point */
void wrm_Stack_delete(wrm_Stack *p);

/* Type-generic pool accessor macros for `pool` (NOT a pointer) and type name `t` */ 

#define wrm_Stack_AS(stack, t) ((t*)((stack).data))
// check that `idx` < `stack.len` first!!
#define wrm_Stack_AT(stack, t, idx) (wrm_Stack_AS(stack, t) + idx)


#endif