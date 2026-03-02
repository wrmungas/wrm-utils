# Development Log

Collection of my thoughts and plans as I add new features or change existing ones. Document design decisions
and version numbers with releases

# Current Plans (0.2)

I want to redo some of the features of 0.1 and expand them for better safety and usability, primarily
the memory module, and create a top-level logging module 

### Overall
Want to rename module namespaces for simplicity
- common: just wrm_
- input: wrm_inp_ (keyboard and mouse input)
- linmath: (not needed, follow cglm style)
- render: wrm_gl_ 
- memory: wrm_mem_
- log: wrm_log_
- gui: wrm_gui_

Future names:
- window: wrm_win_
- file io (including stdin, stdout, stderr): wrm_io_
- os utilities? (multithreading): wrm_os_
- data structures and algorithms: wrm_dsa_
- binary operations: wrm_bin_
- audio: wrm_al_

### Memory
Want to rename/reorganize data structures for more consistency and take advantage of C type checking
- add a new type specifically for data structure indices, `Index`
- rename fields of `Pool` and `Stack` to balance brevity with descriptiveness
- make `Tree` store the Nodes itself, each with Indices to the data, parent, and children
- make a `Bintree` type for built-in binary trees - consider adding `Quadtree` and `Octree` too
- make a `Reflist` type for lists of indices


This will be a cross-project API-breaking change as many functions will now return a wrm_Index instead
of a wrm_Handle

### GUI
Want to rework gui elements organization:
- everything resolves to a set of 2d boxes draw atop each other
- only relevant for the internal GL system that draws these, not the user (directly)
- however, user-visibly system of types must support this logically
- should be as simple as possible

Separate containers and content:
- ultimately, innermost content is either text or an image
- containers determine limits of content alignment, and may have backgrounds/borders

Reference for simplicity: Clay
- flexible containers, horizontal or vertical layout
- simpler: need only be able to draw rectangles
- floating container stacks, but only rooted from a non-floating container stack

Containers:
- `Root`: root of a container tree; may be positioned absolutely or relatively to any element in a different absolute tree
- `Box`: flexible container, positioned within a root or another box; may layout children vertically or horizontally

Alignment:
- `Position`: includes the x and y of a corner, and fields `which` and `from` to determine what corner of the box this refers to and which corner in the container it is relative to
    - may be one of  `{ TOP_LEFT, TOP_CENTER, TOP_RIGHT, CENTER_LEFT, CENTER, CENTER_RIGHT, BOTTOM_LEFT, BOTTOM_CENTER, BOTTOM_RIGHT }`
- `Layout`: rules for internal layout
    - padding: array of top, left, bottom, right values, can be initialized with a `PADDING_ALL()` macro
    - vertical: bool
    - max_width, max_height, min_width, min_height
Content:
- `Text`: 
- `Image`:



# Change Log

### Initial (0.1)

Base state of project
Includes basic modules to make an app:
- common
- memory
- linmath
- render
- input
- gui



