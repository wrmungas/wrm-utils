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
#include "wrm/memory.h"
#include "wrm/render.h"
#include "wrm/input.h"

/*
Constants
*/

enum wrm_gui_Pos_Options { WRM_LEFT = 0, WRM_RIGHT, WRM_CENTER, WRM_TOP, WRM_BOTTOM };

/*
Type declarations
*/

/* union of all gui elements */
typedef union wrm_gui_Element wrm_gui_Element;

typedef struct wrm_gui_Alignment wrm_gui_Alignment;

typedef struct wrm_gui_Properties wrm_gui_Properties;

/* Represents a colored or textured pane as a background for other elements */
typedef struct wrm_Pane wrm_Pane;
/* Represents a box of text; should probably have a semi- or fully transparent parent pane */
typedef struct wrm_Text wrm_Text;

typedef struct wrm_Image wrm_Image;


/*
Type definitions
*/

// implicitly relative to top left; all values in pixels
struct wrm_gui_Alignment {
    i32 x;
    i32 y;
    u32 width; // width in pixels
    u32 height; // height in pixels
    // to add: which point of the element is this referring to, and where on screen is it relative to
    u8 x_is; // which point on the element x is
    u8 x_from; // which point on screen x is relative to
    u8 y_is; // which point on the element y is
    u8 y_from; // which point on screen y is relative to
};

struct wrm_gui_Properties {
    enum { WRM_GUI_TEXT, WRM_GUI_PANE, WRM_GUI_IMAGE} type;
    wrm_gui_Alignment alignment;
    wrm_Tree_Node tree_node;
    bool shown;
    bool children_shown;
};

struct wrm_Text {
    wrm_gui_Properties properties;
    const char *src_text;
    wrm_Handle font;
    wrm_RGBA text_color;
    u32 line_spacing; // pixels between each line
    u32 pixel_limit; // maximum length of each line
    bool wrap; // whether or not to wrap a line upon reaching the pixel limit
};

struct wrm_Pane {
    wrm_gui_Properties properties;
    wrm_RGBA color;
    u32 children;
    u8 child_count;
    bool show_children;
};

struct wrm_Image {
    wrm_gui_Properties properties;
    wrm_Handle image_texture;
};


union wrm_gui_Element {
    wrm_gui_Properties properties;
    wrm_Text text;
    wrm_Pane pane;
    wrm_Image image;
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

// all elements
bool wrm_gui_setAlignment(wrm_Handle element, wrm_gui_Alignment alignment);
bool wrm_gui_setShown(wrm_Handle element, bool shown);
bool wrm_gui_setChildrenShown(wrm_Handle element, bool shown);
bool wrm_gui_addChild(wrm_Handle parent, wrm_Handle child);
bool wrm_gui_removeChild(wrm_Handle parent, wrm_Handle child);

// text box

/* Create a text box gui element */
wrm_Option_Handle wrm_gui_createText(wrm_gui_Properties properties, wrm_Handle font, wrm_RGBA text_color, const char *src, u32 spacing);
 
// pane

/* Create a pane element */
wrm_Option_Handle wrm_gui_createPane(wrm_gui_Properties properties, wrm_RGBA color);

// image

/* Create an image element - will forcibly overwrite the `type` to WRM_GUI_IMAGE in `properties` regardless of what is passed in */
wrm_Option_Handle wrm_gui_createImage(wrm_gui_Properties properties, wrm_Handle texture);

// debug

/* Print out the data for a given alignment */
void wrm_gui_debugAlignment(wrm_gui_Alignment a);
void wrm_gui_debugElement(wrm_Handle element);
/* Create a test image */
wrm_Option_Handle wrm_gui_createTestImage(void);

#endif