#include "wrm/memory.h"

bool wrm_Stack_init(wrm_Stack *s, size_t capacity, size_t element_size, bool auto_reserve)
{
    if(!s) return false;

    s->cap = capacity;
    s->element_size = element_size;
    s->data = calloc(capacity, element_size);

    s->auto_reserve = auto_reserve;
    return s->data;
}

bool wrm_Stack_reserve(wrm_Stack *s, size_t capacity)
{
    if(capacity <= s->cap) return false;

    if(!realloc(s->data, capacity * s->element_size)) { return false; }
    s->cap = capacity;
    return true;
}

bool wrm_Stack_shrink(wrm_Stack *s, size_t capacity)
{
    if(capacity > s->cap) return false;

    if(!realloc(s->data, capacity * s->element_size)) { return false; }
    s->cap = capacity;
    return true;
}

wrm_Option_Handle wrm_Stack_push(wrm_Stack *s)
{
    if(s->len == s->cap) {
        if(!(s->auto_reserve && wrm_Stack_reserve(s, s->cap * WRM_MEMORY_GROWTH_FACTOR))) { 
            return OPTION_NONE(Handle);
        }
    }
    return OPTION_SOME(Handle, s->len++);
}

// force the compiler to emit a symbol

void wrm_Stack_reset(wrm_Stack *s, size_t len);

void wrm_Stack_delete(wrm_Stack *s)
{
    free(s->data);
    s->data = NULL;
    s->element_size = 0;
    s->cap = 0;
    s->len = 0;
}