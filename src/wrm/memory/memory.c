#include "memory.h"

u32 id_counter = 0;

wrm_FUNC(internal_resize, void*, void*, size_t);


// helper to print ascii characters non-literally
void print_escaped(char c) {
    if(
        (c >= '0' && c <= '9')
        || (c >= 'a' && c <= 'z') 
        || (c >= 'A' && c <= 'Z')
    ) {
        printf("%c ", c);
        return;
    }
    char *str;

    switch(c) {
        case '\n':
            str = "\\n";
            break;
        case '\0':
            str = "\\0";
            break;
        case '\t':
            str = "\\t";
            break;
        case '\b':
            str = "\\b";
            break;
        case '\a':
            str = "\\a"; 
            break;
        default:
            str = "  ";
            break;
    }
    printf(str);
}

// helper to print characters from a byte array
void print_chars(
    u8 *data,
    u32 n,
    const char *fmt, 
    const char *pre, 
    const char *delim, 
    const char *post
) {
    if(!data || !n) { return; }

    if(pre) { printf(pre); }

    for(u32 i = 0; i < n; i++) {
        if(wrm_cstr_equals("%c", fmt)) {
            print_escaped(data[i]);
        }
        else {
            printf(fmt, data[i]);
        }

        if(delim && i < n - 1) { printf(delim); }
    }

    if(post) { printf(post); }
    printf("\n");
}

inline void print_ascii_line(mem_Hexdump_Args *args, u32 idx) 
{
    u8 *data = args->data;
    data += idx;
    u32 n = args->bytes_per_line;

    if(idx + n > args->bytes) {
        n = args->bytes - idx;
    }

    print_chars(
        args->data, n, 
        "%c", 
        args->fmt.hex.pre, 
        args->fmt.hex.delim, 
        args->fmt.hex.post
    );
}

inline void print_hex_line(mem_Hexdump_Args *args, u32 idx) 
{
    u8 *data = args->data;
    data += idx;
    u32 n = args->bytes_per_line;
    
    if(idx + n > args->bytes) {
        n = args->bytes - idx;
    }

    print_chars(
        args->data, 
        n, 
        "%x", 
        args->fmt.ascii.pre, 
        args->fmt.ascii.delim, 
        args->fmt.ascii.post
    );
}


void mem_hexdump(mem_Hexdump_Args *args)
{
    if(!args) { return; }

    for(u32 i = 0; i < args->bytes; i += args->bytes_per_line) {
        print_hex_line(args, i);
        if(args->ascii) {
            print_ascii_line(args->data, i);
        }
    }
}

void mem_init(wrm_FUNC(resize, void*, void*, size_t))
{
    internal_resize = resize;
    id_counter = 1;
}