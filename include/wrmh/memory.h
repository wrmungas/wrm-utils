#ifndef WRM_MEMORY_H
#define WRM_MEMORY_H

#include "wrm/common.h"

#define WRM_MEMORY_GROWTH_FACTOR 2

/*
Type declarations
*/

typedef struct wrm_Pool wrm_Pool;
typedef struct wrm_List wrm_List;


/*
Type definitions
*/

struct wrm_Pool {
    size_t element_size;
    size_t cap;
    size_t used;
    void *data;
    bool *is_used;
};


/*
Function declarations
*/

// pool

/* Initialize a pool of `cap` elements of `element_size` size */
void wrm_Pool_init(wrm_Pool *p, size_t cap, size_t element_size);
/* Get an available slot (index of an element) from pool `p` */
wrm_Option_Handle wrm_Pool_getSlot(wrm_Pool *p);
/* Release the slot at `idx` for reuse, if it wasn't already available, in pool `p` */
void wrm_Pool_freeSlot(wrm_Pool *p, wrm_Handle idx);
/* Release the resources associated with pool `p`, which is no longer considered usable after this point */
void wrm_Pool_delete(wrm_Pool *p);
/* Type-generic pool accessor macro for `pool` (NOT a pointer) and type name `t` */ 
#define wrm_Pool_dataAs(pool, t) ((t*)(pool.data))

# ifdef WRM_MEMORY_IMPLEMENTATION

void wrm_Pool_init(wrm_Pool *p, size_t cap, size_t element_size)
{
    if(!p) return;

    p->cap = cap;
    p->element_size = element_size;
    p->used = 0;

    p->data = calloc(cap, element_size);
    p->is_used = calloc(cap, sizeof(bool));
}

wrm_Option_Handle wrm_Pool_getSlot(wrm_Pool *p)
{
    if(p->used == p->cap) {
        if(!(realloc(p->data, p->cap * WRM_MEMORY_GROWTH_FACTOR * p->element_size ) && realloc(p->is_used, p->cap * WRM_MEMORY_GROWTH_FACTOR * p->element_size))) {
            return (wrm_Option_Handle){.exists = false};
        }
        p->cap *= WRM_MEMORY_GROWTH_FACTOR;
        p->is_used[p->used] = true;
        return (wrm_Option_Handle){.exists = true, .Handle_val = p->used++};
    }

    size_t i = 0;
    while(p->is_used[i]) { i++; }
    p->is_used[i] = true;
    p->used++;
    return (wrm_Option_Handle){.exists = true, .Handle_val = i};
}

void wrm_Pool_freeSlot(wrm_Pool *p, wrm_Handle idx)
{
    if(idx >= p->cap) return;
    p->is_used[idx] = false;
    p->used--;
}

void wrm_Pool_delete(wrm_Pool *p)
{
    free(p->data);
    free(p->is_used);

    p->data = NULL;
    p->is_used = NULL;
}
# endif // end header implementation

#endif