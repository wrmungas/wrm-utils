#ifndef WRM_GUI_H
#define WRM_GUI_H
/* --- HEADER DESCRIPTION -----------------------------------------------------

File gui.h

Created Nov 7, 2025
by William R Mungas (wrm)

Last modified Nov 14, 2025

Contributors: 
wrm: creator

DESCRIPTION:
Simple 2d GUI system built on top of the wrm rendering system

FEATURES:
- basic GUI hierarchy: flexible vertical and horizontal Boxes
- leaf nodes: content, either Text or Image
- font loading: wraps FreeType and bakes a font loaded at a given size to a 
    wrm_Texture as an atlas

REQUIREMENTS:
- wrm common functionality
- wrm render and memory; should be initialized after and shut down before both
- FreeType library must be installed/linked with

---------------------------------------------------------------------------- */

#include "common.h"
#include "memory.h"
#include "render.h"
#include "input.h"

#define wrm_NAMESPACE(name) wrm_gui_ ## name

/* --- TYPE DECLARATIONS --------------------------------------------------- */

/* 
Represents the positioning of a root element of a tree,
relative to the screen or other elements 
*/
typedef struct wrm_gui_Position 
wrm_gui_Position;

/* 
Represents the gaps in pixels between a the edges of a box's 
outer and inner bounding rectangles
*/
typedef struct wrm_gui_Padding 
wrm_gui_Padding;

/* 
Represents the rules for how a box should internally compute 
the layout of its children 
*/
typedef struct wrm_gui_Layout
wrm_gui_Layout;

/* 
Represents a rectangle of screen pixels, 
used to define bounding boxes 
*/
typedef struct wrm_gui_Rect
wrm_gui_Rect;

/* Represents a container for other elements */
typedef struct wrm_gui_Box
wrm_gui_Box;

/* Represents text content */
typedef struct wrm_gui_Text 
wrm_gui_Text;

/* Represents image content */
typedef struct wrm_gui_Image 
wrm_gui_Image;

wrm_ENUM(Anchors, u8, extern, 
    top_left, top, top_right, 
    left, center, right, 
    bottom_left, bottom, bottom_right
);

/* --- TYPE DEFINITIONS ---------------------------------------------------- */

struct wrm_gui_Position {
    i16 x;
    i16 y;
    u8 which;
    u8 from;
};

struct wrm_gui_Padding {
    u16 top, left, bottom, right;
};

#define wrm_PAD_ALL(val) (wrm_gui_Padding) {val, val, val, val}

struct wrm_gui_Layout {
    wrm_gui_Padding padding;
    u16 max_width, 
        max_height, 
        min_width, 
        min_height;
    bool vertical;
    u16 child_gap;

};

union wrm_gui_Content {
    
};

struct wrm_gui_Box {
    wrm_Index parent;
    wrm_Index root;
    u8 children;
    union {
        wrm_Index content;
        wrm_Index child;
        wrm_Index children;
    };
};

struct wrm_Text {
    wrm_Index font;
    const char *src_text;
    wrm_RGBA text_color;
    u16 max_width; // maximum width of a line in pixels
    u16 max_height; // maximum height of lines, in pixels
    u8 line_gap; // pixels between each line
    u8 char_gap;
    bool wrap; // whether or not to wrap a line upon reaching the width, or just stop
};

struct wrm_Image {
    wrm_Index image_texture;
    u16 max_width;
    u16 max_height;
};

/* 
Constants
*/

/*
Globals
*/

/*
Function declarations
*/

// module

/* Initialize the gui system */
bool wrm_gui_init(const char *shader_dir);
/* Update gui state based on user input */
void wrm_gui_update(void);
/* draw the most recently updated gui */
void wrm_gui_draw(void);
/* Shut down the gui system */
void wrm_gui_quit(void);

// font

/* Load a font from a given font file */
wrm_Index wrm_gui_loadFont(const char *path);

// content

/* Create an image from a texture */
wrm_Index wrm_gui_createImage(wrm_Index texture);
/* Create a text box element */
wrm_Index wrm_gui_createText(wrm_gui_Text *text);

// box


void wrm_gui_setLayout(wrm_Index box, wrm_gui_Layout *layout);
bool wrm_gui_setContent(wrm_Index box, wrm_Index content);

void wrm_gui_hide(wrm_Index box);
void wrm_gui_show(wrm_Index box);

void wrm_gui_hideChildren(wrm_Index box);
void wrm_gui_showChildren(wrm_Index box);

bool wrm_gui_addChild(wrm_Index box, wrm_Index child);
bool wrm_gui_removeChild(wrm_Index box, wrm_Index child);

// debug

/* Create a test image */
wrm_Index wrm_gui_createTestImage(void);

#endif