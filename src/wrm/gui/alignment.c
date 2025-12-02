#include "gui.h"

// inline - ensure compiler emits a symbol

void wrm_gui_NDC_fromPos(i32 x, i32 y, vec2 dest);
void wrm_gui_NDC_fromDim(i32 w, i32 h, vec2 dest);

inline void wrm_gui_NDC_fromAlignment(wrm_gui_Alignment a, vec2 pos, vec2 dim)
{
    i32 tl_x, tl_y;
    wrm_gui_getTopLeft(a, &tl_x, &tl_y);

    wrm_gui_NDC_fromPos(tl_x, tl_y, pos);
    wrm_gui_NDC_fromDim(a.width, a.height, dim);
}

void wrm_gui_getTopLeft(wrm_gui_Alignment a, i32 *tl_x, i32 *tl_y)
{
    
    i32 x_abs;
    i32 y_abs;

    // first get the absolute position of the given x and y
    switch(a.x_from) {
        case WRM_LEFT:
            x_abs = a.x;
            break;
        case WRM_RIGHT:
            x_abs = wrm_window_width - a.x;
            break;
        case WRM_CENTER:
            // explicit fallthrough
        default:
            // CENTER and any other values are treated as relative to the screen center
            x_abs = wrm_window_width / 2 + a.x;
            break;
    }

    switch(a.y_from) {
        case WRM_TOP:
            y_abs = a.y;
            break;
        case WRM_BOTTOM:
            y_abs = wrm_window_height - a.y;
            break;
        case WRM_CENTER:
            // explicit fallthrough
        default:
            // CENTER and any other values are treated as relative to the screen center
            y_abs = wrm_window_height / 2 + a.y;
            break;
    }

    // then get x and y of the top-left corner, relative to the given position
    if(tl_x) {
        switch(a.x_is) {
            case WRM_LEFT:
                *tl_x = x_abs;
                break;
            case WRM_RIGHT:
                *tl_x = x_abs - a.width;
                break;
            case WRM_CENTER:
                // explicit fallthrough
            default:
                // CENTER and any other values are treated as relative to the screen center
                *tl_x = x_abs - a.width / 2;
                break;
        }
    }

    if(tl_y) {
        switch(a.y_is) {
            case WRM_TOP:
                *tl_y = y_abs;
                break;
            case WRM_BOTTOM:
                *tl_y = y_abs - a.height;
                break;
            case WRM_CENTER:
                // explicit fallthrough
            default:
                // CENTER and any other values are treated as relative to the screen center
                *tl_y = y_abs - a.height / 2;
                break;
        }
    }
}

bool wrm_gui_setAlignment(wrm_Handle element, wrm_gui_Alignment alignment)
{
    if(!wrm_Pool_isValid(&wrm_gui_elements, element)) return false;

    wrm_gui_Element *e = wrm_Pool_at(&wrm_gui_elements, element);
    if(!e) return false;
    e->properties.alignment = alignment;
    return true;
}

void wrm_gui_debugAlignment(wrm_gui_Alignment a)
{
    i32 tl_x, tl_y;
    wrm_gui_getTopLeft(a, &tl_x, &tl_y);

    vec2 pos;
    vec2 dim;
    wrm_gui_NDC_fromPos(tl_x, tl_y, pos);
    wrm_gui_NDC_fromDim(a.width, a.height, dim);

    const char *strings[5] = {
        "LEFT", "RIGHT", "CENTER", "TOP", "BOTTOM"
    };


    printf(
        "Original (pixels): x %d, y %d, w %d, h %d\n"
        "Alignment: from: x %s y %s as: x %s y %s\n"
        "Top-Left (pixels): x %d, y %d\n"
        "NDC: x %f, y %f, w %f, h %f\n", 
        a.x, a.y, a.width, a.height, 
        strings[a.x_from], strings[a.y_from], strings[a.x_is], strings[a.y_is],
        tl_x, tl_y, 
        pos[WRM_X], pos[WRM_Y], dim[WRM_X], dim[WRM_Y]
    );
}