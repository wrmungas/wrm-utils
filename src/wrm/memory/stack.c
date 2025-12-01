#include "wrm/memory.h"

bool wrm_Stack_init(wrm_Stack *s, size_t capacity, size_t element_size, bool auto_reserve)
{
    if(!s) return false;

    s->len = 0;
    s->cap = capacity;
    s->e_size = element_size;
    s->data = calloc(capacity, element_size);

    s->auto_reserve = auto_reserve;
    return s->data;
}

bool wrm_Stack_reserve(wrm_Stack *s, size_t capacity)
{
    if(!s || capacity < s->cap) return false;

    void *temp = realloc(s->data, capacity * s->e_size);
    if(!temp) { return false; }
    s->data = temp;
    s->cap = capacity;
    return true;
}

bool wrm_Stack_shrink(wrm_Stack *s, size_t capacity)
{
    if(!s || capacity > s->cap || capacity < s->len) return false;

    void *temp = realloc(s->data, capacity * s->e_size);
    if(!temp) { return false; }
    s->data = temp;
    s->cap = capacity;
    return true;
}

wrm_Option_Handle wrm_Stack_push(wrm_Stack *s)
{
    if(!s) return OPTION_NONE(Handle);
    if(s->len == s->cap) {
        if(!(s->auto_reserve && wrm_Stack_reserve(s, s->cap * WRM_MEMORY_GROWTH_FACTOR))) { 
            return OPTION_NONE(Handle);
        }
    }
    return OPTION_SOME(Handle, s->len++);
}

void wrm_Stack_delete(wrm_Stack *s, void (*delete)(void *element))
{
    if(!s || !s->data) { return; }

    if(delete) {
        for(u32 i = 0; i < s->len; i++) {
            delete(wrm_Stack_at(s, i));
        }
    }
    
    free(s->data);
    s->data = NULL;
    s->e_size = 0;
    s->cap = 0;
    s->len = 0;
}

// force the compiler to emit a symbol

void wrm_Stack_reset(wrm_Stack *s, size_t len);
void *wrm_Stack_offsetAt(wrm_Stack *s, wrm_Handle idx, size_t offset);
void *wrm_Stack_at(wrm_Stack *s, wrm_Handle idx);