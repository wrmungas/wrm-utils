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
// represents a continually-growing stack of elements: may be used as an arena, only reset on a manual call to reset()
typedef struct wrm_Stack wrm_Stack;
// represents a stack- and pool-friendly, handle-based node for linking types into trees
typedef struct wrm_Tree_Node wrm_Tree_Node;
// represents a stack- and pool-friendly, handle-based tree
typedef struct wrm_Tree wrm_Tree;

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

struct wrm_Tree_Node {
    u32 parent;
    u32 children;
    u8 child_count;
    bool root;
};

struct wrm_Tree {
    enum { WRM_POOL, WRM_STACK } src_type;
    void *src; // source pool or stack : NOT owned by the tree, and only the Tree_Nodes within are modified
    wrm_Pool child_lists;  // auxiliary pool of child lists: owned by the tree
    size_t child_limit; // basically child_lists->element_size / sizeof(u32)
    size_t tn_offset; // byte offset of Tree_Node struct inside objects
};



/*
Function declarations
*/

// pool and stack

/* Type-generic data accessor macro for `mem` (NOT a pointer) and type name `t` */ 
#define wrm_data_AS(mem, t) ((t*)((mem).data))

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
Calls the provided `delete` 
Returns `true` if the operation was successful
Never automatically called
*/
bool wrm_Pool_shrink(wrm_Pool *p, size_t capacity);
/* Get an available slot (index of an element) from pool `p` */
wrm_Option_Handle wrm_Pool_getSlot(wrm_Pool *p);
/* Check that the slot at a given index is valid for pool `p`: p is not NULL, the index is within p, and the element at index is in use */
inline bool wrm_Pool_isValid(wrm_Pool *p, wrm_Handle idx)
{
    return p && idx < p->cap && p->is_used[idx];
}
/* Release the slot at `idx` for reuse, if it wasn't already available, in pool `p` */
inline void wrm_Pool_freeSlot(wrm_Pool *p, wrm_Handle idx)
{
    if(!wrm_Pool_isValid(p, idx)) { return; }
    p->is_used[idx] = false;
    p->used--;
}
/* Get a safe void* to a location `offset` bytes from the start of the element at `idx` */
inline void *wrm_Pool_offsetAt(wrm_Pool *p, wrm_Handle idx, size_t offset)
{
    return (wrm_Pool_isValid(p, idx) && (offset < p->element_size))  ? (u8*)p->data + idx * p->element_size + offset : NULL;
}
/* Get a safe void* to a location in a pool; returns NULL if the idx is invalid (out-of-bounds or freed slot)*/
inline void *wrm_Pool_at(wrm_Pool *p, wrm_Handle idx)
{
    return wrm_Pool_offsetAt(p, idx, 0);
}
/* 
Release the resources associated with pool `p`
Iterates over stack and calls the provided `delete()` on each element in use, then frees the pool's memory
Pool is is no longer considered usable after this point  
*/
void wrm_Pool_delete(wrm_Pool *p, void (*delete)(void *element));


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
inline void *wrm_Stack_offsetAt(wrm_Stack *s, wrm_Handle idx, size_t offset)
{
    return (s && idx < s->len && (offset < s->element_size))  ? (u8*)s->data + idx * s->element_size + offset : NULL;
}
/* Safe stack at function, returns NULL if the index is invalid (beyond top of stack) */
inline void *wrm_Stack_at(wrm_Stack *s, wrm_Handle idx)
{
    return wrm_Stack_offsetAt(s, idx, 0);
}
/* 
Release the resources associated with stack `s`
Iterates over stack and calls the provided `delete()` on each element, then free the stack memory
Stack is is no longer considered usable after this point 
*/
void wrm_Stack_delete(wrm_Stack *p, void (*delete)(void *element));


// tree 

/* Initializes a tree off of a source pool/stack */
bool wrm_Tree_init(wrm_Tree *tree, void *source, u32 type, size_t tn_offset, size_t child_limit);
/* Wrapper over stack and pool access functions for genericity */
inline void *wrm_Tree_offsetAt(wrm_Tree *tree, u32 idx, size_t offset)
{
    if(!tree || !tree->src ) { return NULL; }
    switch(tree->src_type) {
        case WRM_STACK:
            return wrm_Stack_offsetAt((wrm_Stack*)tree->src, idx, offset);
        case WRM_POOL:
            return wrm_Pool_offsetAt((wrm_Pool*)tree->src, idx, offset);
        default:
            return NULL;
    }
}
/* 
Associates a child and parent, if possible 
`src` is the pool that parent and child are both indices into
`child_lists` is the auxiliary pool that stores lists of children; the maximum size of a child list is implicitly taken from this
`tn_offset` is the offset of the Tree_Node struct within whatever elements the tree is made of
*/
bool wrm_Tree_addChild(wrm_Tree *tree, u32 parent, u32 child);
/* 
Dissociates a child and parent, if possible 
`src` is the pool that parent and child are both indices into
`child_lists` is the auxiliary pool that stores lists of children; the maximum size of a child list is implicitly taken from this
`tn_offset` is the offset of the Tree_Node struct within whatever elements the tree is made of
*/
bool wrm_Tree_removeChild(wrm_Tree *tree, u32 parent, u32 child);
/* 
Checks whether the parent has `child` in its child list
`src` is the pool that parent and child are both indices into
`child_lists` is the auxiliary pool that stores lists of children
`tn_offset` is the offset of the Tree_Node structure within whatever elements the tree is made of
*/
bool wrm_Tree_hasChild(wrm_Tree *tree, u32 parent, u32 child);
/* 
Checks whether the parent has `child` in its child list
`src` is the pool that parent and child are both indices into
`child_lists` is the auxiliary pool that stores lists of children
`tn_offset` is the offset of the Tree_Node structure within whatever elements the tree is made of
*/
void wrm_Tree_makeRoot(wrm_Tree *tree, u32 node);
/*
Dissociates all nodes and frees the lists of children
*/
void wrm_Tree_delete(wrm_Tree *tree);



#endif