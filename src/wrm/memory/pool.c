#include "memory.h"

// gets a pointer to a slot in the pool
inline void *get_slot(mem_Pool *p, u32 idx) 
{
    return (void*)((u8*)(p->data) + p->item_size * idx);
}

/*
Gets the 'next' value (index of the next free slot) stored at a slot in
the pool (assumes the slot is a free slot)
*/
inline u32 get_next(mem_Pool *p, u32 idx) 
{
    return *((u32*)get_slot(p, idx));
}
/* Checks whether a slot in the pool is free */
inline bool is_free(mem_Pool *p, u32 idx)
{
    u32 this = p->free_head;
    while(this < p->item_cap) {
        if(this == idx) {
            return true;
        }
        this = get_next(p, this);
    }
    return false;
}

/*
Sets the 'next' value (index of the next free slot) stored at a slot in
the pool (assumes the slot is a free slot)
*/
inline void set_next(mem_Pool *p, u32 idx, u32 next) 
{
    *((u32*)get_slot(p, idx)) = next;
}

bool mem_init_pool(
    mem_Pool *p, 
    void *buf, 
    size_t isize, 
    size_t icap,
    bool final
) {
    if(!id_counter) {
        return false;
    }

    *p = (mem_Pool) {
        .data = buf,
        .id = id_counter++,
        .item_cap = icap,
        .item_size = isize,
        .free_cnt = icap,
        .free_head = 0,
        .final = final        
    };

    // initialize free list - indices beyond p->item_cap are considered invalid
    for(u32 i = 0; i < icap; i++) {
        set_next(p, i, i + 1);
    }
}

mem_Ref mem_palloc(mem_Pool *p)
{
    if(!p) { return mem_null; }
    if(p->free_cnt == 0) { 
        if(p->final || !mem_resize) {
            return mem_null;
        }
        void *new_data = mem_resize(
            p->data, 
            p->item_cap * MEM_GROWTH_FACTOR * p->item_size 
        );

        if(!new_data) {
            return mem_null;
        }

        p->data = new_data;
        size_t old_cap = p->item_cap;
        p->item_cap *= MEM_GROWTH_FACTOR;
        p->free_cnt += p->item_cap - old_cap;
    }

    mem_Ref ret = (mem_Ref){p->id, p->free_head};

    u32 *head = get_slot(p, p->free_head);
    p->free_head = *head;
    p->free_cnt--;

    return ret;
}

void *mem_pderef(mem_Pool *p, mem_Ref r)
{
    if(!p || !r.src || r.src != p->id) { return NULL; }

    return get_slot(p, r.idx);
}

void mem_pfree(mem_Pool *p, mem_Ref *r, void (*delete)(void *))
{
    if(!p || !r || !r->src || r->src != p->id) {
        return;
    }
    u32 *slot = get_slot(p, r->idx);
    if(delete) { delete(slot); }

    *slot = p->free_head;
    p->free_head = r->idx;
    p->free_cnt++;

    *r = mem_null;
}

void mem_delete_pool(
    mem_Pool *p, 
    wrm_FUNC(delete, void, void*), 
    wrm_FUNC(free, void, void*)
) {
    if(!p || !p->id) {
        return;
    }

    // call delete on all slots currently in-use (this might be tough, or at least slow)
    for(u32 i = 0; i < p->item_cap; i++) {
        if(!is_free(p, i) && delete) {
            delete(get_slot(p, i));
        }
    }

    p->id = 0;
    if(free) { free(p->data); }
}