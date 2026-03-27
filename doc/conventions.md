# Conventions

A summary of coding conventions used in this project, so that I can be 
consistent.

## Lines

Keep lines to 80 characters. Not only does this look better in terminal 
editors, this looks much better on my screen when I tile windows side-by-side,
which I do frequently.

Keep things that super easily fit on one line to just that. For example, 
one-off single-step `if` statements are fine to go on one line like so:
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
- variables and functions are in `snake_case`
- compile-time constants and macros are in `SCREAMING_SNAKE_CASE`
- structs, unions, and enums are generally typedef'ed and in `PascalCase`
- all names at global scope are prefixed with the module, like `wrm_`, `mem_`, 
    or `gfx_`: especially important for user-visible names

## Comments

Comment where necessary to describe the reasoning behind complex code (mainly
for future me). Always comment function declarations to document the purpose, 
arguments, return value, and anything non-obvious about behavior. 

Use comments to structure files into sections as well (see below)

## Source Code Structure

Use comments to structure files into logical sections.

Headers should always be organized into the following sections:
- Description
- Other includes
- Compile-time constants and macros
- Type declarations
- Type definitions
- Global variables (marked explicitly as `extern`)
- Function declarations

Implementation files should always be organized into the following sections:
- Global variables (this file is where they live)
- Helper declarations
- Function definitions
- Helper definitions

## Directory Structure
The project follows this structure:
`/`
- `bin/`
    - static library binary output location (empty until built)
- `build/`
    - intermediate object files used in build process (empty until built)
- `doc/`
    - Markdown documentation for the project
- `include/`
    - `(other)/`
        - headers for other code
    - `wrm/`
        - headers for wrm code
- `resources/`
    - resources used in testing
- `src/`
    - `glad/`
        - source for glad
    - `stb/`
        - file defining stb implementation
    - `shaders/`
        - default shader files
    - `wrm/`
        - `(module)/`
            - internal header for each module
            - source files implementing a group of related operations
- `test/`
    - `bin/`
        - test binary output location (empty until built)
    - common test header
    - test source files for each module


## Commits

Commit messages are all implicitly related to the content of the branch they are on

Commits should be done frequently for small sets of related changes to a few files

Each commit should have a prefix that relates to what the commit accomplishes:
- `[H]` (header):
    - create/edit header files
- `[I]` (implementation): 
    - create/edit implementation files (takes precedence over header)
- `[T]` (test): 
    - create/edit test files (should be separate from I)
- `[D]` (documentation): 
    - changes to comments in source files 
    - changes to text structure in source files (not the actual code content)
    - changes to markdown documentation
- `[S]` (start/skeleton): 
    - add empty/boilerplate files for a new feature
- `[R]` (refactor): 
    - change a bunch of existing files to restructure something 
    - doesn't need to resolve all of the issues introduced by this
- `[B]` (build): 
    - alter build files