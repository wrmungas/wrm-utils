#include "gui.h"

FT_Library wrm_ft_library;
wrm_Stack wrm_fonts;

wrm_Pool wrm_gui_elements;

static const char *WRM_GUI_SHADER_V_NAME = "ui.vert";
static const char *WRM_GUI_SHADER_F_NAME = "ui.frag";
wrm_Handle wrm_gui_shader;

// wrm_Pool wrm_gui_child_lists;

// file-internal helper declarations
static wrm_Option_Handle wrm_gui_loadShader(const char *shader_dir);

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

    wrm_Option_Handle shader = wrm_gui_loadShader(shader_dir);
    if(!shader.exists) {
        wrm_error("GUI", "init()", "failed to initialize elements pool");
        return false;
    }
    wrm_gui_shader = shader.val;

    return true;
}

void wrm_gui_update(void)
{

}

void wrm_gui_draw(void)
{

    wrm_gui_prepareElements();

    
}

void wrm_gui_quit(void)
{

}

// file-internal helpers

static wrm_Option_Handle wrm_gui_loadShader(const char *shader_dir)
{
    size_t dir_len = strlen(shader_dir);
    // names are the same length
    size_t name_len = strlen(WRM_GUI_SHADER_V_NAME);

    // include slash and .vert
    size_t len = dir_len + 1 + name_len;
    // include null terminator
    char *vert_path = malloc(len + 1);
    char *frag_path = malloc(len + 1);

    sprintf(vert_path, "%s/%s", shader_dir, WRM_GUI_SHADER_V_NAME);
    sprintf(frag_path, "%s/%s", shader_dir, WRM_GUI_SHADER_F_NAME);

    printf("vert: %s frag: %s\n", vert_path, frag_path);

    char *vert = wrm_readFile(vert_path);
    char *frag = wrm_readFile(frag_path);

    if(!vert || !frag) {
        return OPTION_NONE(Handle);
    }

    wrm_Option_Handle result; 
    result = wrm_render_createShader(vert, frag, true, true);
    free(vert);
    free(frag);
    free(vert_path);
    free(frag_path);
    
    return result;
}