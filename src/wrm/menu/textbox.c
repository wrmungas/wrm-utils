#include "menu.h"

// helpers

void wrm_menu_createMeshFromText(const char *src);

// user-visible

wrm_Option_Handle wrm_menu_createTextBox(wrm_Text_Box *data, float x, float y, u32 width, u32 height, wrm_Handle parent)
{
    wrm_Option_Handle slot = wrm_Pool_getSlot(&wrm_elements);

    
    if(!slot.exists) {
        if(wrm_menu_settings.errors) { wrm_error("Menu", "createTextBox()", "failed to find memory for text box"); }
        return slot;
    }

    if(parent && !wrm_menu_exists(parent, "createTextBox()", "element", "(parent value)")) {
        wrm_Pool_freeSlot(&wrm_elements, slot.val);
        return OPTION_NONE(Handle);
    }

    wrm_Menu_Element *tb = wrm_Pool_dataAs(wrm_elements, wrm_Menu_Element) + slot.val;

    *tb = (wrm_Menu_Element){
        .type = WRM_TEXT_BOX,
        .self.text_box = *data,
        .x = x,
        .y = y,
        .width = width,
        .height = height,
        .child_count = 0,
        .children = {0, 0, 0, 0},
        .visible = true,
        .show_children = false,
    };

    // create associated GUI data
    wrm_Option_Handle model = wrm_menu_createRenderData(tb);
    if(!model.exists) {
        if(wrm_menu_settings.errors) { wrm_error("Menu", "createTextBox()", "failed to create rendering data for the text box"); }
        return model;
    }

    tb->model = model.val;
    
    // wrm_menu_updateTextBox(slot.Handle_val, data);

    if(parent) {
        wrm_menu_addChild(parent, slot.val); // need to do this after creating render data so models are updated
    }

    return slot;
}

bool wrm_menu_updateTextBox(wrm_Handle textbox, wrm_Text_Box *t)
{
    return true;
}

// module internal

void wrm_menu_createMeshFromText(const char *src)
{

}