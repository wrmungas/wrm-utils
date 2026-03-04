# Conventions

A summary of coding conventions used in this project, so that I can be 
consistent.

## Lines

Keep lines to 80 characters. Not only does this look better in terminal 
editors, this looks much better on my screen when I tile windows side-by-side,
which I do frequently.

Keep things that super easily fit on one line to just that. For example, 
one-off single-step if statements are fine to go on one line like so:
```c
if(condition) { action(); }
```
A function definition's opening brace goes on the next line, like so:
```c
bool create()
{
    // ... implementation
}
```
For functions for which the declaration does not fit, move all arguments to a vertical list indented once, like so:
```c
int long_function(
    void *arg1,
    int super_long_arg,
    bool really_long_argument_flag,
    int arg2,
    int arg3 
); 
```
In this case the definition braces can start on the same
line as the closing paren:
```c
int long_function(
    void *arg1,
    int super_long_arg,
    bool really_long_argument_flag,
    int arg2,
    int arg3 
) {
    // ... implementation
}
```

## Names

In general, prefer the shortest name that is still descriptive. Only fully
abbreviate module prefixes or super common programming terms like `idx`, 
`ptr`, `arg`, `num`, `src`, or `cnt`. These kinds of things are preferred to be
appended to another name to give it more context, like `mesh_args`. Type names 
should be spelled out in full, minus such common abbreviations - having context 
is worth the time/space taken up by the typing the full name. Do not be afraid 
of long names, but neither try to proliferate them. 

For specific C constructs:
- variables are generally in `snake_case`
- compile-time constants and macros are in `SCREAMING_SNAKE_CASE`
- structs, unions, and enums are generally typedef'ed and in `PascalCase`
- functions are generally in `camelCase`
- all names at global scope are prefixed with the module, like `wrm_`, `mem_`, 
    or `gfx_`: this especially important for user-visible names

## Comments

Comment where necessary to describe the reasoning behind complex code (mainly
for future me). Always comment function declarations to document the purpose, 
arguments, return value, and anything non-obvious about behavior. 

Use comments to structure files into sections as well:

## Structure

Use comments to structure files into logical sections.

Headers should always be organized into the following sections:
- Description
- Compile-time constants and macros
- Type declarations
- Type definitions
- Global variables (marked as `extern`)