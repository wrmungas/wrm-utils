# Development Log

Collection of my thoughts and plans as I add new features or change existing ones. Document design decisions
and version numbers with releases

# Current Plans (0.2)

I want to redo some of the features of 0.1 and expand them for better safety and usability, primarily
the memory module, and create a top-level logging module 

### Memory
Want to rename/reorganize data structures for more consistency and take advantage of C type checking
- add a new type specifically for data structure indices, `wrm_Index`, with 
- rename fields of `wrm_pool` and `wrm_stack` to balance brevity with descriptiveness
- this will be a cross-project API-breaking change as many functions will now return a wrm_Index instead
of a wrm_Handle



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



