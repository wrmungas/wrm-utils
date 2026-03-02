#ifndef WRM_MEMORY_H
#define WRM_MEMORY_H

/*
File memory.h

Created  Nov 14, 2025 
by William R Mungas (wrm)

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

// represents a safe index into a data structure
typedef struct wrm_Index wrm_Index;
// represents a pool of elements
typedef struct wrm_Pool wrm_Pool;
// represents a continually-growing stack of elements: may be used as an arena, only reset on a manual call to reset()
typedef struct wrm_Stack wrm_Stack; 
// represents a tree node with exactly two children
typedef struct wrm_Binary_Tree_Node;
// represents a tree of nodes with exactly two children
typedef struct wrm_Binary_Tree;

// TODO: possibly add BST?

// represents a tree node with an arbitrary number of children < 256
typedef struct wrm_Tree_Node wrm_Tree_Node;
// represents a stack- and pool-friendly, handle-based tree
typedef struct wrm_Tree wrm_Tree;

/*
Type definitions
*/

struct wrm_Index {
    u32 val;
    u32 src;
};

struct wrm_Pool {
    void *items; // source array of elements
    u8 *in_use; // bit vector to track which slots are available

    size_t item_size; // size in bytes of each slot/item
    size_t cap; // number of total slots for items in the pool
    size_t used; // number of slots that are taken up

    u32 id; // id for indices

    bool reserve; // whether or not to call wrm_Pool_reserve() automatically when no slots are available
};

struct wrm_Stack {
    void *data; // source array of elements

    size_t item_size; // size in bytes of each item in the stack
    size_t cap;
    size_t len;

    u32 id; // id for indices

    bool reserve; // whether or not to call wrm_List_reserve() automatically when no slots are available
};

struct wrm_Binary_Node {
    wrm_Index parent;
    wrm_Index self;
    wrm_Index left;
    wrm_Index right;
};

struct wrm_Binary_Tree {
    wrm_Pool nodes;
    wrm_Pool *src;
};

struct wrm_Tree_Node {
    wrm_Index parent; // index of this node's parent in the source pool
    wrm_Index self; // index of this node in the source pool
    wrm_Index child_list; // index of this node's child list in the child_lists pool;
    u8 child_count;
};

struct wrm_Tree {
    wrm_Pool *src; // source pool: NOT owned, only referenced
    wrm_Pool nodes; // pool of nodes in the tree: owned
    wrm_Pool child_lists;  // auxiliary pool of child lists: owned
    size_t child_limit; // maximum number of children each node can have
};

wrm_OPTION(wrm_Index, Index);


/*
Function declarations
*/

// cast generic data member to pointer to type

#define wrm_data_AS(buf, t) ((t*)((buf).data))

/*
helper to get the value at a position in a bit vector 
*/ 
inline bool wrm_bitAt(u8 *bit_vec, u32 idx)
{
    // TODO: implement
}

// pool

/* 
Initialize a pool of `capacity` elements of `element_size` bytes each
Returns `true` if the operation was successful
`reserve` determines whether the pool will automatically allocate space for new elements 
*/
bool wrm_Pool_init(wrm_Pool *p, size_t capacity, size_t element_size, bool reserve);
/* 
Ensure that pool `p` has room for `capacity` total elements by GROWING ONLY 
Returns `true` if the operation was successful
*/
bool wrm_Pool_reserve(wrm_Pool *p, size_t capacity);
/* 
Get an available slot (index of an element) from pool `p` 
*/
wrm_Index wrm_Pool_getSlot(wrm_Pool *p);
/* 
Check that the slot at a given index is valid for pool `p`
- p is not NULL
- the index is within p
- the element at the index is in use 
*/
inline bool wrm_Pool_isValid(wrm_Pool *p, wrm_Index idx)
{
    return p 
    && (idx.src) 
    && (idx.src == p->id) 
    && (idx.val < p->cap) 
    && p->in_use[idx.val];
}
/* 
Release the slot at `idx` for reuse, if it wasn't already available, in pool `p` 
*/
inline void wrm_Pool_freeSlot(wrm_Pool *p, wrm_Index idx)
{
    if(!wrm_Pool_isValid(p, idx)) { return; }
    p->in_use[idx.val] = false;
    p->used--;
}
/* Get a safe void* to a location `offset` bytes from the start of the element at `idx`; returns NULL if `p` is NULL, `idx` is invalid, or `offset` is too big */
inline void *wrm_Pool_offsetAt(wrm_Pool *p, wrm_Index idx, size_t offset)
{
    return (wrm_Pool_isValid(p, idx) && (offset < p->item_size))  ? (u8*)p->data + idx.val * p->item_size + offset : NULL;
}
/* Get a safe void* to a location in a pool; returns NULL if `p` is NULL or `idx` is invalid (out-of-bounds or freed slot) */
inline void *wrm_Pool_at(wrm_Pool *p, wrm_Index idx)
{
    return wrm_Pool_offsetAt(p, idx, 0);
}
/* 
Release the resources associated with pool `p`
Iterates over the pool contents
If `delete()` is not null, it is called on each in-use element
After iteration, frees the pool's memory
`p` is no longer considered usable after this point  
*/
void wrm_Pool_delete(wrm_Pool *p, wrm_FUNCTION(delete, void, void *element));


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
wrm_Index wrm_Stack_push(wrm_Stack *s);
/* Roll back stack `s` to `len`; all elements beyond `len` are now considered invalid */
inline void wrm_Stack_reset(wrm_Stack *s, size_t len)
{
    if(len > s->len) return;
    s->len = len;
}
/* Get a safe void* to a location `offset` bytes from the start of the element at `idx`; returns NULL if `s` is NULL, `idx` is invalid, or `offset` is too big */
inline void *wrm_Stack_offsetAt(wrm_Stack *s, wrm_Index idx, size_t offset)
{
    return (s && (idx.val < s->len) && (offset < s->item_size))  ? (u8*)s->data + idx.val * s->item_size + offset : NULL;
}
/* Get a safe void* to the location at `idx` in stack `s` returns NULL if `s` is NULL or the index is invalid (beyond top of stack) */
inline void *wrm_Stack_at(wrm_Stack *s, wrm_Index idx)
{
    return wrm_Stack_offsetAt(s, idx, 0);
}
/* 
Release the resources associated with stack `s`
Iterates over stack contents
If `delete()` is not null, it is called on each element 
Then frees the stack memory
Stack is no longer considered usable after this point 
*/
void wrm_Stack_delete(wrm_Stack *p, wrm_FUNCTION(delete, void, void*));


// tree 

/* 
Initializes a tree from a source buffer (from a pool/stack)
Creates an auxiliary pool to hold the lists of each node's children
`auto_reserve` determines whether the children pool can resize automatically to fit demand, and should be `true` unless memory is constrained
*/
bool wrm_Tree_init(wrm_Tree *tree, wrm_Pool *src, size_t offset, size_t child_limit, bool auto_reserve);
/* simplified tree node accessor */
inline wrm_Tree_Node *wrm_Tree_at(wrm_Tree *tree, wrm_Index idx)
{
    if(!tree || !tree->src ) { return NULL; }
    return wrm_Pool_offsetAt(tree->src, idx, tree->offset);
}
/* Associates a child and parent, if possible */
bool wrm_Tree_addChild(wrm_Tree *tree, u32 parent, u32 child);
/* Dissociates a child and parent, if possible */
bool wrm_Tree_removeChild(wrm_Tree *tree, u32 parent, u32 child);
/* Checks whether the parent has `child` in its child list */
bool wrm_Tree_hasChild(wrm_Tree *tree, u32 parent, u32 child);
/* Makes a node a root in the tree: if it has a parent, orphan it */
bool wrm_Tree_makeRoot(wrm_Tree *tree, u32 node);
/* Dissociates all nodes and frees the lists of children */
void wrm_Tree_delete(wrm_Tree *tree);
/* print the contents of the tree node */
void wrm_Tree_debugNode(wrm_Tree_Node *tn, wrm_Tree *tree);



#endif