# Conventions

A summary of coding conventions used in this project, so that I can be 
consistent.

## Lines

Keep lines to 80 characters. Not only does this look better in terminal 
editors, this looks much better on my screen when I tile windows side-by-side,
which I do frequently.

In general, keep things that super easily fit on one line to just that; more complex statements should be split across multiple lines. Specific constructs are discussed below.

## `if` Statements

Single-line `if` statements are ok for simple, singular actions executed, like so:
```c
if(condition) { action(); }
```
Always use braces: this removes any possibility of future ambiguity/bugs and takes almost no effort. For `if`s which have more than a simple, single statement, the opening brace goes on the same line as the closing paren in the conditional, and the closing brace goes on the next line after the last statement, like so:
```c
if(condition) {
    action1();
    action2();
    action3();
}
```

For `if`s which have a complex conditional expression that makes the line too long, group main-level conditions with parentheses (and really any conditions which mix `&&` and `||`); then split the conditions across lines by boolean operators, keeping these at the start of each line, like so:
```c
if( (a && b && c)
    || (d && (a || b))
) {
    // body ...
}
```

The closing paren and opening body brace go on the next line after the conditional ends.


## Functions

For functions where the signature does not fit on one line, split it up much like the
`if` statements:
```c
int long_function(
    void *arg1,
    int super_long_arg,
    bool really_long_argument_flag,
    int arg2,
    int arg3 
); 
```

A function definition's opening brace goes on the next line, like so:
```c
bool create()
{
    // ... implementation
}
```

For long function signatures, the closing paren and opening brace can go on the same line:
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

Use comments to structure files into sections as well. Headers should have the following sections, in order:
- Description
- Compile-time constants and macros
- Type declarations
- Type definitions
- Global variables (marked as `extern`)
- Function declarations (and inline definitions)

Internal headers should follow a similar format.

Implementation files should have the following sections:
- Internal constants
- Internal type declarations
- Internal type definitions
- Global variables
- Internal/helper function declarations
- External function definitions
- Internal/helper definitions

## Structure

The project is organized into different levels of granularity. Each logical 'module' the user can see is defined by an *external* header file in `include/wrm`. The header file provides the key types and the public function interface for the module. 

Each module has a corresponding directory under `src/wrm/<module-name>`. Inside this directory is an *internal* header file, which includes the external one and defines anything shared across all the implementation files; and the implementation files themselves, which
each organize a sub-domain of the module's types/functions. Each implementation file should focus on functions that act on one of the module's types or handle one of the module's internal data structures.

## Git

Commit often, for simple changes. Messages should have a prefix detailing the type of change, such as `[Imp]` for implementation, `[Doc]` for documentation, or `[Int]` for interface (header file) changes.

There are two core branches, `main` and `dev`. Feature branches are created on `dev`, where new features or changes are completed in isolation before merging back into `dev`. Additional branches can be made to integrate groups of features or changes with the rest of the project. `dev` is only pushed to main at a version bump.

In version 0.xx the project is still considered unstable: API-breaking changes will not increase the major version number until 1.0, which will start once I feel the main feature set and core API are stable. 

After this point, the project will use semantic versioning of the form `major.minor.patch`. Fixes to existing content will bump the patch number; introduction of new features without breaking the API will bump the minor number; and introductions or changes that do break the API will bump the major number.


