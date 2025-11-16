#ifndef WRM_GUI_H
#define WRM_GUI_H

/*

File gui.h

Created Nov 7, 2025
by William R Mungas (wrm)

Version: 0.1.0
(Last modified Nov 14, 2025)

DESCRIPTION:
Simple guiing system built on top of the wrm rendering system

PROVIDES
- basic GUI types - panes, text boxes, buttons, sliders
- font loading; stored as bitmap texture atlas and character info table
- hierarchical 2d element 

REQUIREMENTS:
- FreeType library must be installed/linked with
*/

#include "wrm/common.h"
#include "wrm/render.h"
#include "wrm/input.h"

/*
Type declarations
*/

/* union of all gui elements */
typedef union wrm_gui_Element wrm_gui_Element;

typedef struct wrm_gui_Alignment wrm_gui_Alignment;

typedef struct wrm_gui_Header wrm_gui_Header;

/* Represents a colored or textured pane as a background for other elements */
typedef struct wrm_Pane wrm_Pane;
/* Represents a box of text; should probably have a semi- or fully transparent parent pane */
typedef struct wrm_Text wrm_Text;


/*
Type definitions
*/

struct wrm_gui_Alignment {
    float x; // x position 
    float y; // y position
    float width; // width in pixels
    float height; // height in pixels
    enum {TOP_LEFT, TOP_RIGHT, BOTTOM_LEFT, BOTTOM_RIGHT, CENTER } position;

};

struct wrm_gui_Header {
    enum { WRM_GUI_TEXT, WRM_GUI_PANE} type;
    wrm_gui_Alignment alignment;
    u32 model;
    u32 parent;
    bool visible;
};

struct wrm_Text {
    wrm_gui_Header properties;
    wrm_Handle font;
    u32 text_len;
    const char *text;
};

struct wrm_Pane {
    wrm_gui_Header properties;
    u32 children;
    u8 child_count;
    bool show_children;
};


union wrm_gui_Element {
    wrm_gui_Header properties;
    wrm_Text text;
    wrm_Pane pane;
};


/* 
Constants
*/

extern wrm_Handle WRM_GUI_NONE;

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
wrm_Option_Handle wrm_gui_loadFont(const char *path);

// text box

/* Create a text box gui element */
wrm_Option_Handle wrm_gui_createText(wrm_Text *t);
/* Update a text box gui element */
bool wrm_gui_updateText(wrm_Handle text, wrm_Text *t);
 
// pane

/* Create a pane element */
wrm_Option_Handle wrm_gui_createPane(wrm_Pane *p, u32 x, u32 y, u32 width, u32 height, wrm_Handle parent);



#endif