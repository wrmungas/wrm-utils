#ifndef WRM_MENU_H
#define WRM_MENU_H

/*

File wrm-common.h

Created Nov 7, 2025
by William R Mungas (wrm)

Version: 0.1.0
(Last modified Nov 7, 2025)

DESCRIPTION:
Provides some simple ui/menu primitives, to be used with
wrm-render and wrm-input

PROVIDES
- basic UI types
- text renderin

REQUIREMENTS:
- FreeType library must be installed/linked with
*/

#include "wrm/common.h"
#include "wrm/render.h"
#include "wrm/input.h"

/*
Type declarations
*/

/* Represents a colored or textured pane as a background for other elements */
typedef struct wrm_Pane wrm_Pane;
/* Represents a box of text; should probably have a semi- or fully transparent parent pane */
typedef struct wrm_Text_Box wrm_Text_Box;
/* Represents a button element */
typedef struct wrm_Button wrm_Button;
/* Represents a slider element */
typedef struct wrm_Slider wrm_Slider;
/* Generic element: includes general element data and data specific to the element type */
typedef struct wrm_Menu_Element wrm_Menu_Element;
/* Union of all element types */
typedef union wrm_Menu_Element_Any wrm_Menu_Element_Any;

/*
Type definitions
*/

struct wrm_Text_Box {
    wrm_Handle font;
    u32 text_len;
    void (*on_click)(void *); // click callback
    const char *text;
};

struct wrm_Pane {
    bool moveable;
};

union wrm_Menu_Element_Any {
    wrm_Text_Box text_box;
};

struct wrm_Menu_Element {
    enum {WRM_TEXT_BOX, WRM_PANE, WRM_BUTTON, WRM_SLIDER, WRM_TOGGLE} type;
    wrm_Menu_Element_Any self;
    float x; // x position of top left corner on screen, in NDC
    float y; // y position of top left corner on screen, in NDC
    u32 width; // width in pixels
    u32 height; // height in pixels
    u32 model;
    u32 parent;
    u32 children[WRM_MODEL_CHILD_LIMIT];
    u8 child_count;
    bool visible;
    bool show_children;
};


/* 
Constants
*/

extern wrm_Handle WRM_MENU_NONE;

/*
Globals
*/

/*
Function declarations
*/

// module

/* Initialize the menuing system */
bool wrm_menu_init();
/* Update menu state based on user input */
void wrm_menu_update(void);
/* Shut down the menu system */
void wrm_menu_quit(void);

// font

/* Load a font from a given font file */
wrm_Option_Handle wrm_menu_loadFont(const char *path);

// text box

/* Create a text box menu element */
wrm_Option_Handle wrm_menu_createTextBox(wrm_Text_Box *t , float x, float y, u32 width, u32 height, wrm_Handle parent);
/* Update a text box menu element */
bool wrm_menu_updateTextBox(wrm_Handle textbox, wrm_Text_Box *t);

// pane

/* Create a pane element */
wrm_Option_Handle wrm_menu_createPane(wrm_Pane *p, u32 x, u32 y, u32 width, u32 height, wrm_Handle parent);

// general elements

/* Add one menu element to another as a child */
bool wrm_menu_addChild(wrm_Handle parent, wrm_Handle child);
/* Remove the relationship between two menu elements, orphaning the child */
bool wrm_menu_removeChild(wrm_Handle parent, wrm_Handle child);


#endif