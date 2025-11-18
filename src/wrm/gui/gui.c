#include "gui.h"

FT_Library wrm_ft_library;
wrm_Stack wrm_fonts;

wrm_Pool wrm_gui_elements;
wrm_Stack wrm_gui_tbd; // elements to be drawn


static const char *WRM_GUI_TEXT_SHADER_NAME = "ui-text";
static const char *WRM_GUI_IMAGE_SHADER_NAME = "ui-image";
static const char *WRM_GUI_PANE_SHADER_NAME = "ui-pane";



wrm_Handle wrm_gui_text_shader;
wrm_Handle wrm_gui_image_shader;
wrm_Handle wrm_gui_pane_shader;

// wrm_Pool wrm_gui_child_lists;

// file-internal helper declarations
static void wrm_gui_prepareElements(void);
static bool wrm_gui_createDefaultShaders(const char *shader_dir);

// user-visible

bool wrm_gui_init(const char *shader_dir)
{
    
    if(FT_Init_FreeType(&wrm_ft_library)) {
        wrm_error("GUI", "init()", "failed to initialize FreeType");
        return false;
    }

    if(!wrm_Stack_init(&wrm_fonts, WRM_GUI_INITIAL_ELEMENTS_CAPACITY, sizeof(wrm_Font), true)) {
        wrm_error("GUI", "init()", "failed to initialize fonts list");
        return false;
    }
    if(!wrm_Pool_init(&wrm_gui_elements, WRM_GUI_INITIAL_ELEMENTS_CAPACITY, sizeof(wrm_gui_Element), true)) {
        wrm_error("GUI", "init()", "failed to initialize elements pool");
        return false;
    }
    if(!wrm_Stack_init(&wrm_gui_tbd, WRM_GUI_INITIAL_ELEMENTS_CAPACITY, sizeof(wrm_Handle), true)) {
        wrm_error("GUI", "init()", "failed to initialize elements list");
    }

    wrm_gui_initQuad();

    if(!wrm_gui_createDefaultShaders(shader_dir)) {
        wrm_error("GUI", "init()", "failed to initialize gui shaders");
        return false;
    }

    return true;
}

void wrm_gui_update(void)
{

}

void wrm_gui_draw(void)
{
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);

    wrm_gui_prepareElements();

    if(wrm_render_debug_frame) {
        printf("\nGUI (2D) PASS (%zu element%s to be drawn):\n", wrm_gui_tbd.len, wrm_gui_tbd.len == 1 ? "" : "s");
    }

    for(u32 i = 0; i < wrm_gui_tbd.len; i++) {
        wrm_Handle idx = *wrm_Stack_AT(wrm_gui_tbd, wrm_Handle, i);
        wrm_gui_Element *e = wrm_Pool_AT(wrm_gui_elements, wrm_gui_Element, idx);

        if(wrm_render_debug_frame) {
            wrm_gui_debugElement(idx);
        }

        switch(e->properties.type) {
            case WRM_GUI_TEXT:
                wrm_gui_drawText((wrm_Text*)e);
                break;
            case WRM_GUI_PANE:
                wrm_gui_drawPane((wrm_Pane*)e);
                break;
            case WRM_GUI_IMAGE:
                wrm_gui_drawImage((wrm_Image*)e);
                break;
            default:
                // do nothing
                break;
        }
    }

    
}

void wrm_gui_quit(void)
{

}

void wrm_gui_debugElement(wrm_Handle element)
{
    if(!wrm_gui_exists(element)) return;
    wrm_gui_Element *e = wrm_Pool_AT(wrm_gui_elements, wrm_gui_Element, element);
   
    const char *strings[3] = { "TEXT",  "PANE", "IMAGE"};
    printf("GUI Element [%u] (%s): \n", element, strings[e->properties.type]);

    wrm_gui_debugAlignment(e->properties.alignment);
    // TODO: add switch for each element type's debug() function
}

// module internal

bool wrm_gui_exists(wrm_Handle e); // inline


// file-internal helpers

static bool wrm_gui_createDefaultShaders(const char *shader_dir)
{
    wrm_render_Format format = {
        .col = false,
        .tex = true,
        .per_pos = 2
    };

    wrm_Option_Handle result; 
    result = wrm_render_loadAndCreateShader(shader_dir, WRM_GUI_TEXT_SHADER_NAME, format);
    if(!result.exists) {
        wrm_error("GUI", "createDefaultShaders()", "failed to create text shader!");
        return false;
    }
    wrm_gui_text_shader = result.val;

    // reuse format
    result = wrm_render_loadAndCreateShader(shader_dir, WRM_GUI_IMAGE_SHADER_NAME, format);
    if(!result.exists) {
        wrm_error("GUI", "createDefaultShaders()", "failed to create image shader!");
        return false;
    }
    wrm_gui_image_shader = result.val;

    format = (wrm_render_Format) {
        .col = true,
        .tex = false,
        .per_pos = 2
    };
    result = wrm_render_loadAndCreateShader(shader_dir, WRM_GUI_PANE_SHADER_NAME, format);
    if(!result.exists) {
        wrm_error("GUI", "createDefaultShaders()", "failed to create pane shader!");
        return false;
    }
    wrm_gui_pane_shader = result.val;

    return true;
}


static void wrm_gui_prepareElements(void)
{
    wrm_Stack_reset(&wrm_gui_tbd, 0);

    for(u32 i = 0; i < wrm_gui_elements.cap; i++) {
        // if the element exists and is visible, add its handle to the top of the tbd stack
        if(wrm_gui_elements.is_used[i] && wrm_Pool_AT(wrm_gui_elements, wrm_gui_Element, i)->properties.visible) {
            wrm_Option_Handle top = wrm_Stack_push(&wrm_gui_tbd);
            if(top.exists) {
                *wrm_Stack_AT(wrm_gui_tbd, wrm_Handle, top.val) = i;
            }
        }
    }
}