# wrm-utils
---
A framework for creating graphical applications and games in C. Expands on the functionality of SDL to provide a
more complete foundation for 3d graphics. 

I may expand/rewrite this to C++ in the future :)

The project in its current state has almost all features I desired for the initial version. I still
have a few changes in mind, including:
- a rework of error handling and print functions
- a rework of some of the memory structures 
- string buffering in `string.h` (currently unimplemented)
- updating the `wrmh/` folder for a header-only version of the project (do not use in current state)
- a rework of the Makefile to include targets to build the project to static (.a) and dynamic (.so) library files, and to build 
the test programs by linking to these

Longer-term features I would like to consider include:
- more complex model format to include skeletal animation
- more involved gui items composed of the different primitives
- menu layout computation that is more flexible and automatic
- much more comprehensive documentation and user guidance

# Features
---
Features are grouped by the following headers:
- `render.h`: 3d rendering primitives (shaders, textures, meshes, and models)
- `input.h`: mouse and keyboard input handling
- `gui.h`: very simple 2d menu primitives (panes, images, and text)
- `memory.h`: basic data structures based on allocating large blocks of memory and only growing when necessary (stack, pool, tree)

# Usage
---
The only current way to use wrm-utils is to build from source and link the 
link it into other projects. You should copy the contents of the `include/wrm/` directory to your
project for the necessary headers. At the moment the Makefile doesn't support building the project to a 
library, but that is the next planned addition. 

The `include/wrmh/` directory is intended for a header-only version of the project, and is not currently
implemented. DO NOT USE IT.

# Dependencies
--- 
This project links with the FreeType and SDL libraries, and includes header-only files from `stb` and `cglm`.
These all must be in your include path. This project also uses the C standard library, including the standard
math library. There is not an easy way around this that I know of, since I use specifically sized integer types
and several standard functions, particularly the `malloc` family. I would like to add the flexibility to avoid 
linking with the standard library in the future, but my knowledge here is not extensive. One of my goals with 
design was to limit allocations as much as possible, so this should not be a major issue, but there is still
much room for improvement!