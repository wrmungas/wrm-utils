#include "wrm/memory.h"


bool wrm_Pool_init(wrm_Pool *p, size_t cap, size_t element_size, bool auto_reserve)
{
    if(!p) return false;

    p->cap = cap;
    p->e_size = element_size;
    p->used_cnt = 0;

    p->data = calloc(cap, element_size);
    p->in_use = calloc(cap, sizeof(bool));
    p->auto_reserve = auto_reserve;

    return p->data && p->in_use;
}

wrm_Option_Handle wrm_Pool_getSlot(wrm_Pool *p)
{
    if(p->used_cnt == p->cap) {
        if(!(p->auto_reserve && wrm_Pool_reserve(p, p->cap * WRM_MEMORY_GROWTH_FACTOR))) {
            return OPTION_NONE(Handle);
        }
        p->in_use[p->used_cnt] = true;
        memset(wrm_Pool_at(p, p->used_cnt), 0, p->e_size); // clear any prior data to zero
        return OPTION_SOME(Handle, p->used_cnt++);
    }

    size_t i = 0;
    while(p->in_use[i]) { i++; }
    p->in_use[i] = true;
    p->used_cnt++;
    memset(wrm_Pool_at(p, i), 0, p->e_size);
    return OPTION_SOME(Handle, i);
}

bool wrm_Pool_reserve(wrm_Pool *p, size_t capacity)
{
    if(capacity >= WRM_POOL_MAX_CAPACITY) return false;
    // reallocate
    void *temp = realloc(p->data, capacity * p->e_size );
    if(!temp) { return false; }
    p->data = temp;

    temp = realloc(p->in_use, capacity * sizeof(bool));
    if(!temp ) { return false; }
    p->in_use = temp;
    
    p->cap = capacity;
    return true;
}

bool wrm_Pool_shrink(wrm_Pool *p, size_t capacity)
{
    if(capacity < p->used_cnt) return false;

    // allocate a new, smaller pool
    wrm_Pool new_pool;
    wrm_Pool_init(&new_pool, capacity, p->e_size, p->auto_reserve);

    // copy (shallow) all the old elements over
    for(u32 i = 0; i < p->cap; i++) {
        if(p->in_use[i]) {
            void *src = wrm_Pool_at(p, i); 
            wrm_Option_Handle result = wrm_Pool_getSlot(&new_pool);
            if(!result.exists) {
                wrm_error("Pool", "shrink()", "could not copy all data from the old to the new buffer!!");
                return false;
            }
            void *dest = wrm_Pool_at(&new_pool, result.val);
            memcpy(dest, src, p->e_size);
        }
    }

    // just to be sure
    if(new_pool.used_cnt != p->used_cnt) {
        wrm_error("Pool", "shrink()", "could not copy all data from the old to the new buffer!!");
        return false;
    }

    // delete (shallow) old resources and swap
    wrm_Pool_delete(p, NULL);
    *p = new_pool;
    return true;
}

void wrm_Pool_delete(wrm_Pool *p, void (*delete)(void *element))
{
    if(!p || !p->data || !p->in_use) { return; }

    if(delete) {
        for(u32 i = 0; i < p->cap; i++) {
            if(p->in_use[i]) delete(wrm_Pool_at(p, i)); 
        }
    }
    

    free(p->data);
    free(p->in_use);

    p->data = NULL;
    p->in_use = NULL;

    p->used_cnt = 0;
    p->e_size = 0;
    p->cap = 0;
}

// force the compiler to emit a symbol for these

bool wrm_Pool_isValid(wrm_Pool *p, wrm_Handle idx);
void wrm_Pool_freeSlot(wrm_Pool *p, wrm_Handle idx);
void *wrm_Pool_offsetAt(wrm_Pool *p, wrm_Handle idx, size_t offset);
void *wrm_Pool_at(wrm_Pool *p, wrm_Handle idx);
