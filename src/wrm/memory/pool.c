#include "wrm/memory.h"


bool wrm_Pool_init(wrm_Pool *p, size_t cap, size_t element_size, bool auto_reserve)
{
    if(!p) return false;

    p->cap = cap;
    p->element_size = element_size;
    p->used = 0;

    p->data = calloc(cap, element_size);
    p->is_used = calloc(cap, sizeof(bool));
    p->auto_reserve = auto_reserve;

    return p->data && p->is_used;
}

wrm_Option_Handle wrm_Pool_getSlot(wrm_Pool *p)
{
    if(p->used == p->cap) {
        if(!(p->auto_reserve && wrm_Pool_reserve(p, p->cap * WRM_MEMORY_GROWTH_FACTOR))) {
            return OPTION_NONE(Handle);
        }
        p->cap *= WRM_MEMORY_GROWTH_FACTOR;
        p->is_used[p->used] = true;
        return OPTION_SOME(Handle, p->used++);
    }

    size_t i = 0;
    while(p->is_used[i]) { i++; }
    p->is_used[i] = true;
    p->used++;
    return OPTION_SOME(Handle, i);
}

bool wrm_Pool_reserve(wrm_Pool *p, size_t capacity)
{
    if(capacity >= WRM_POOL_MAX_CAPACITY) return false;
    if(!(realloc(p->data, capacity * p->element_size ) && realloc(p->is_used, capacity * sizeof(bool)) ) ) { return false; }
    
    p->cap = capacity;
    return true;
}

bool wrm_Pool_shrink(wrm_Pool *p, size_t capacity)
{
    if(capacity < p->used) return false;

    // allocate a new, smaller pool
    wrm_Pool new_pool;
    wrm_Pool_init(&new_pool, capacity, p->element_size, p->auto_reserve);

    // copy all the old elements over
    for(u32 i = 0; i < p->cap; i++) {
        if(p->is_used[i]) {
            void *src = wrm_Pool_AT(*p, u8, i * p->element_size); 
            wrm_Option_Handle result = wrm_Pool_getSlot(&new_pool);
            if(!result.exists) {
                wrm_error("Pool", "shrink()", "could not copy all data from the old to the new buffer!!");
                return false;
            }
            void *dest = wrm_Pool_AT(new_pool, u8, result.val);
            memcpy(dest, src, p->element_size);
        }
    }

    // just to be sure
    if(new_pool.used != p->used) {
        wrm_error("Pool", "shrink()", "could not copy all data from the old to the new buffer!!");
    }

    // delete old resources and swap
    wrm_Pool_delete(p);
    *p = new_pool;
    return true;
}

// force the compiler to emit a symbol for these

bool wrm_Pool_isValid(wrm_Pool *p, wrm_Handle idx);

void wrm_Pool_freeSlot(wrm_Pool *p, wrm_Handle idx);

void wrm_Pool_delete(wrm_Pool *p)
{
    free(p->data);
    free(p->is_used);

    p->data = NULL;
    p->is_used = NULL;

    p->used = 0;
    p->element_size = 0;
    p->cap = 0;
}
