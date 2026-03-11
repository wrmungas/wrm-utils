#ifndef WRM_COMMON_H
#define WRM_COMMON_H
/* --- HEADER DESCRIPTION -----------------------------------------------------
File common.h

Created  Oct 29, 2025
by William R Mungas (wrm)

Last modified Nov 14, 2025

Contributors: 
wrm: creator

DESCRIPTION:
Standard baseline for my wrm utility headers, used by all of the others.
These utilities are intended as a set to be included from a single 'wrm' 
folder, and all will use this one for common definitions, C standard includes,
and other generally useful functions and types.

Anything more niche and unnecessary/not used by the other utilities will be
found in wrm-misc instead.

FEATURES:
- several C standard library #includes to establish/wrap baseline functionality
- bit-sized integer types: u8-u64 and i8-i64
- pair and option types with declaration macros
- function pointer type macro
- a printf-like fail() function to exit with a given code and a stderr message
- a string matching function to match a string against a set and return an 
    integer, for usage with switch statements; and a macro for slightly 
    cleaner syntax

REQUIREMENTS:
Uses C standard library
Must link with C standard math (-lm)

---------------------------------------------------------------------------- */

/* standard set of includes */

#include <stdio.h> // print and file io functions
#include <stdbool.h> // booleans! thank fuck
#include <stddef.h> // size_t, NULL! thank fuck
#include <stdint.h> // int<n>_t, uint<n>_t
#include <string.h> // string comparison, mostly so I can wrap it myself
#include <stdarg.h> // so I can do unnecessarily complicated things :P
#include <math.h> // so I can do fucking math

/* --- SIZED INTEGER TYPES ------------------------------------------------- */

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

/* --- UNSIGNED INTEGER SHORTHANDS ----------------------------------------- */

typedef unsigned char uchar;
typedef unsigned short ushort;
typedef unsigned int uint;
typedef unsigned long ulong;

/* --- REFERENCE TYPE ------------------------------------------------------ */

// offers indirection with extra safety and flexibility
typedef struct wrm_Ref wrm_Ref;

struct wrm_Ref {
    u32 src;
    u32 idx;
};

/* --- TYPE MACROS --------------------------------------------------------- */

/*
For function pointers
*/
#define wrm_FUNC(name, return_type, ...) return_type (*name)(__VA_ARGS__)

/*
Create an enum as a struct with the value names as fields
Allows me to namespace with dot syntax rather than creating a lot of
ugly globally-visible macros (besides this one, I suppose)
*/
#define wrm_ENUM(name, type, qualifiers, ...) struct _wrm_ ## name ## _Enum \
{ type __VA_ARGS__; }; qualifiers struct _wrm_ ## name ## _Enum const wrm_ \
## name

/* 
Defines the values of each field of an enum struct
IMPORTANT: values must match the number it was declared with and must be unique
*/
#define wrm_ENUM_VALUES(name, ...) struct __wrm_ ## name ## _Enum const wrm_ \
## name = { __VA_ARGS__ }

/* --- PAIR AND OPTION TYPES ----------------------------------------------- */

/*
Define a new Pair type

Must specify:
- the full type names (t1, t2)
- the variable-friendly names (t1_name, t2_name)

ex: 
`wrm_PAIR(int, int, const char*, cstr)`
results in 
`wrm_Pair_int_cstr { int int_val; const char * cstr_val; }`
*/
#define wrm_PAIR(t1, t1_name, t2, t2_name) typedef struct wrm_Pair_ ## \
t1_name ## _ ## t2_name { \
    t1 t1_name ## _val; \
    t2 t2_name ## _val; \
} wrm_Pair_ ## t1_name ## _ ## t2_name
/*
Define a new Option type

Must specify:
- the exact contained type name (t)
- the name you want to use in the option type name (t_name)

ex:
`wrm_OPTION(const char *, cstr)`
results in
`wrm_Option_cstr { bool exists; const char * val; }`
*/
#define wrm_OPTION(t, t_name) typedef struct wrm_Option_ ## t_name { \
    bool exists; \
    t val; \
} wrm_Option_ ## t_name
/*
For returning option types with no contained value from functions
*/
#define wrm_NONE(t_name) (wrm_Option_ ## t_name){.exists = false}
/*
For returning option types with a contained value from functions
*/
#define wrm_SOME(t_name, v) (wrm_Option_ ## t_name){.exists = true, .val = v}
/*
Define a new list struct type

Must specify:
- the exact contained type name (t)
- the name you want to use in the list type name (t_name)

ex
`wrm_LIST(char *, cstr)`
results in
`wrm_List_cstr { u32 cap, u32 len, char **data; }`
*/
#define wrm_LIST(t, t_name) typedef struct wrm_List_ ## t_name { \
    u32 cap; \
    u32 cnt; \
    t *data; \
} wrm_List_ ## t_name

/* --- UTILITY FUNCTIONS --------------------------------------------------- */

/*
For module-specific errors, print detailed error statement
*/
void wrm_error(const char *module, const char *function, 
    const char *format, ... )
// GCC PRINTF VARARGS WARNINGS
__attribute__((format(printf, 3, 4)));
/* 
For critical program failure
Exits with the given code and error message
Works like printf
*/
void wrm_fail(u8 code, const char *module, const char *function, 
    const char *format, ...)
// GCC PRINTF VARARGS WARNINGS
__attribute__((format(printf, 4, 5)));

/*
Compares two c-style strings: simple wrapper around strcmp for usability
*/
bool wrm_cstr_equals(const char *str1, const char *str2);

/*
Compares two c-style strings for up to `n` characters
Returns `true` if they are equal
*/
bool wrm_cstrn_equals(size_t n, const char *str1, const char *str2);
/*
Matches the src string with the first of the options
Returns 1 if the first option is matched, 2 if the second...
Returns zero if no options are matched
Can use this for switch statements on a set of strings
*/
int wrm_cstr_match(const char *src, const char **opts, size_t n_opts);
/*
Matches the src string with the first of the options
Comparisons are limited to the first n characters
Returns 1 if the first option is matched, 2 if the second...
Returns zero if no options are matched
Can use this for switch statements on a set of strings
*/
int wrm_cstrn_match(size_t len, const char *src, const char **opts, 
    size_t n_opts );
/*
Possibly dangerous match syntax macro for several c-style strings
Returns 1 if the first option is matched, 2 if the second...
Returns zero if no options are matched
Can use this for switch statements on a set of strings
*/
#define wrm_MATCH(C, ...) wrm_str_match(C, \
    (const char*[]){__VA_ARGS__}, \
    sizeof( (const char*[]){__VA_ARGS__} ) / sizeof (const char*) )
/*
Reads the contents of the file at `path` to a string
The string is allocated by `malloc` and should be freed by the caller
*/
char *wrm_readFile(const char *path);

#endif // end include guards