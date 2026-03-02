# wrm-utils
---
A framework for creating graphical applications and games in C.

I may expand/rewrite this to C++ in the future :)

The project in its current state has almost all features I desired for the initial version. I still
have a few changes in mind, including:
- a rework of error handling and print functions (introduce a `log` module)
- extracting window creation to a `window` module to prevent everything depending on `render`
- a rework of some of the memory structures 
- extension of `input` to include things like controllers and joysticks
- adding string types and buffering with `string.h` (currently unimplemented)
- updating the `wrmh/` folder for a header-only version of the project (do not use in current state)

# Features
---
See [modules](doc/modules.md) for information on the features offered by this project.

As an overview:
- `common`: provides baseline integer types and macros used across the rest of the project
- `memory`: fundamental data structures
- `linmath`: expands on cglm for some useful functions
- `input`: by-frame keyboard and mouse input polling
- `render`: 3d graphics primitives: shaders, textures, meshes, models, and cameras
- `gui`: 2d GUI primitives: panes, textboxes, and images

Short-term planned features include:
- addition of a `log` module to remove dependence on `common` for logging functions
- reworking `memory` data structures (and adding a few new ones) and creating a distinct `Index` type (rather than just a `u32` typedef; yes, this is a band-aid over my choice to use a language with the type-safety of C)


Longer-term plans include:
- reworking the model format to support things like skeletal animation
- adding the ability to directly load .obj files to `wrm_Model`s
- extracting window creation to a `window` or `os` module to prevent everything depending on `render`
- improving `gui` layout control (now that I'm more familiar with CSS, I have a lot of ideas); possibly using the Clay layout library
- improving `input` to handle things outside of keyboard-and-mouse PC world; I'd like to be able to use controllers as well!
- reworking the Makefile to build to a shared library (DONE!)
- reworking the project to minimize dependencies and possibly include its own OS compatibility layer

# Dependencies
---
This project depends on a few packages:
- SDL: specifically SDL2
- FreeType

Also, this implicitly depends on the standard C library:
- I'd like to move away from this in the future if people want control over allocations in particular
- my policy in creating this is to use arena-based structures especially since this is oriented toward games
    - typical style is to perform large allocations up-front for the major data structures that will persist for most of the app's lifetime
    - the initial size, whether to allow reallocations, and the reallocation policy are up to the user
    - testing and profiling will suggest what initial memory sizes work for either minimizing reallocations or forbidding them altogether
but currently I just use `malloc`/`calloc`/`realloc` minimally with arena-type memory structures, and `free` everything 
when module `quit` functions are called

# Usage
---
The only current way to use wrm-utils is to build from source as a static library and link it directly
into your projects. You should copy the contents of `include/wrm/` to your
project for the necessary headers. 

The provided Makefile *should* build the project to a static library, which you can add in your dependencies folder and link to during your project's compilation. In the future I plan to support building to and installing as a shared library, but I'm learning as I go.
