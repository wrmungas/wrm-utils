#include "menu.h"

/* 
Constants
*/

/*
Globals
*/

// window parameters
int wrm_window_width;
int wrm_window_height;

// vertical scale of a single pixel in Normalized Device Coordinates
float ndc_scale_h;
// horizontal scale of a pixel in Normalized Device Coordinates
float ndc_scale_w;

wrm_Settings wrm_menu_settings;

// menu elements
wrm_Pool wrm_elements; // pool of element data

// text rendering
wrm_Stack wrm_fonts;
FT_Library wrm_ft_library;

wrm_Mesh_Buffer wrm_mesh_buf;

// user visible

bool wrm_menu_init(wrm_Settings *s)
{
    wrm_menu_settings = *s;
    
    wrm_Pool_init(&wrm_elements, 10, sizeof(wrm_Menu_Element), true);

    wrm_Stack_init(&wrm_fonts, 10, sizeof(wrm_Font), true);
    wrm_mesh_buf = (wrm_Mesh_Buffer) {
        .vtx_cap = 100,
        .vtx_len = 0,
        .positions = calloc(100 * 3, sizeof(float)),
        .uvs = calloc(100 * 2, sizeof(float)),
        .colors = calloc(100 * 4, sizeof(float)),
        .idx_cap = 50,
        .idx_len = 0,
        .indices = calloc(50, sizeof(u32)),
    };

    int error = FT_Init_FreeType(&wrm_ft_library);
    if(error) {
        if(wrm_menu_settings.errors) { wrm_error("Menu", "init()", "failed to initialize FreeType"); }
        return false;
    }

    // reserve element 0 as root of the hierarchy
    wrm_elements.used++;
    wrm_elements.is_used[0] = true;
    wrm_Menu_Element *root = wrm_Pool_dataAs(wrm_elements, wrm_Menu_Element);

    root->parent = 0;
    root->visible = false;
    root->show_children = true;

    SDL_Window *w = wrm_render_getWindow();
    SDL_GetWindowSize(w, &wrm_window_width, &wrm_window_height);
    ndc_scale_h = 2.0f / wrm_window_height;
    ndc_scale_w = 2.0f / wrm_window_width;

    if(wrm_menu_settings.test) {
        wrm_menu_createTestElement();
    }

    return true;
}

void wrm_menu_update(void)
{

}

void wrm_menu_quit(void)
{
    for(u32 i = 0; i < wrm_fonts.len; i++) {
        free(wrm_Stack_dataAt(wrm_fonts, wrm_Font, i)->glyph_set);
    }
    free(wrm_fonts.data);

    free(wrm_mesh_buf.positions);
    free(wrm_mesh_buf.uvs);
    free(wrm_mesh_buf.colors);
    free(wrm_mesh_buf.indices);

    FT_Done_FreeType(wrm_ft_library);
}

bool wrm_menu_addChild(wrm_Handle parent, wrm_Handle child)
{
    char * caller = "addChild()";
    if(!(wrm_menu_exists(parent, caller, "", "(parent)") && wrm_menu_exists(child, caller, "", "(child)"))) {
        return false;
    }

    wrm_Menu_Element *m = wrm_Pool_dataAs(wrm_elements, wrm_Menu_Element);

    if(!wrm_render_addChild(m[parent].model, m[child].model)) {
        wrm_error("Menu", "addChild()", "failed to sync models and menu elements - see render error");
        return false;
    }

    if(m[parent].child_count == WRM_MODEL_CHILD_LIMIT) {
        wrm_error("Menu", "addChild()", "cannot add another child to menu element [%u] (limit reached!)\n", parent); 
        return false;
    }

    if(m[child].parent) {
        wrm_error("Menu", "addChild()", "cannot add parent to menu element [%u] (already has parent!)\n", child); 
        return false;
    }

    for(u8 i = 0; i < m[parent].child_count; i++) {
        if(m[parent].children[i] == child) {
            wrm_error("Menu", "addChild()", "cannot add child [%u] to menu element [%u] (already added!)\n", child, parent); 
            return false;
        }
    }

    if(parent) m[parent].children[m[parent].child_count++] = child;
    m[child].parent = parent;
    return true;
}

bool wrm_menu_removeChild(wrm_Handle parent, wrm_Handle child) 
{
    char * caller = "removeChild()";
    char * type = "element";
    if(!(wrm_menu_exists(parent, caller, type, "(parent)") && wrm_menu_exists(child, caller, type, "(child)"))) {
        return false;
    }

    wrm_Menu_Element *m = wrm_Pool_dataAs(wrm_elements, wrm_Menu_Element);

    if(!wrm_render_removeChild(m[parent].model, m[child].model)) {
        wrm_error("Menu", "removeChild()", "failed to sync models and menu elements - see render error");
        return false;
    }

    if(m[parent].child_count == 0) {
        wrm_error("Menu", "removeChild()", "cannot remove child from model [%u] (has no children!)\n", parent); 
        return false;
    }

    if(!m[child].parent) {
        wrm_error("Menu", "removeChild()", "cannot remove parent from model [%u] (has no parent!)\n", child); 
        return false;
    }

    if(m[child].parent != parent) {
        wrm_error("Menu", "removeChild()", "cannot remove parent [%u] from model [%u] (has a different parent!)\n", parent, child); 
        return false;
    }   

    
    
    // find the child
    u8 idx;
    for(idx = 0; idx < m[parent].child_count; idx++) {
        if(m[parent].children[idx] == child) break;
    }
    if(idx == WRM_MODEL_CHILD_LIMIT) {
        wrm_error("Menu", "removeChild()", "cannot remove child [%u] from model [%u] (already removed!)", child, parent); 
        return false;
    }

    m[child].parent = 0;

    m[parent].children[idx] = 0;
    m[parent].child_count--;
    
    while(idx < m[parent].child_count) {
        m[parent].children[idx] = m[parent].children[idx + 1];
        idx++;
    }
    m[parent].children[idx] = 0;

    return true;
}




// Module internal

void wrm_menu_resetMeshBuffer(wrm_Mesh_Buffer *m);


bool wrm_menu_addVertex(wrm_Mesh_Buffer *m, wrm_Vertex *v, bool colors, bool uvs)
{    
    u32 i = m->vtx_len;

    if(m->vtx_len == m->vtx_cap) {
        bool success = 
            realloc(m->positions, m->vtx_cap * 2 * 3 * sizeof(float)) && 
            realloc(m->colors, m->vtx_cap * 2 * 4 * sizeof(float)) && 
            realloc(m->uvs, m->vtx_cap * 2 * 2 * sizeof(float));

        if(!success) {
            wrm_error("Menu", "addVertex():", "failed to allocate space for %u vertices in mesh buffer!", m->vtx_cap * 2);
            return false;
        }
        m->vtx_cap *= 2;
    }


    m->positions[3 * i] = v->pos[0];
    m->positions[3 * i + 1] = v->pos[1];
    m->positions[3 * i + 2] = v->pos[2];

    if(colors) {
        m->colors[4 * i] = v->col[0];
        m->colors[4 * i + 1] = v->col[1];
        m->colors[4 * i + 2] = v->col[2];
        m->colors[4 * i + 3] = v->col[3];
    }
    
    if(uvs) {
        m->uvs[2 * i] = v->uv[0];
        m->uvs[2 * i + 1] = v->uv[1];
    }
    
    m->vtx_len++;
    return true;
}

bool wrm_menu_addIndices(wrm_Mesh_Buffer *m, u32* indices, u8 count)
{
    if(m->idx_len + count >= m->idx_cap) {
        if(!realloc(m->indices, m->idx_cap * 2 * sizeof(u32))) {
            wrm_error("Menu", "addIndices():", "failed to allocate space for %u indices in mesh buffer!", m->vtx_cap * 2);
            return false;
        }
        m->idx_cap *= 2;
    }
    for(u8 i = 0; i < count; i++) {
        m->indices[m->idx_len++] = indices[i];
    }
    return true;
}

bool wrm_menu_addQuad(wrm_Mesh_Buffer *m, wrm_Quad *q)
{
    if(!q || !m) return false;

    if(!(q->has_col || q->has_uv)) {
        wrm_error("Menu", "addQuad()", "quad must have uvs or colors");
        return false;
    }


    u32 idx = m->vtx_len;
    // add four vertices
    bool success = 
        wrm_menu_addVertex(&wrm_mesh_buf, &q->tl, q->has_col, q->has_uv) &&
        wrm_menu_addVertex(&wrm_mesh_buf, &q->tr, q->has_col, q->has_uv) &&
        wrm_menu_addVertex(&wrm_mesh_buf, &q->bl, q->has_col, q->has_uv) &&
        wrm_menu_addVertex(&wrm_mesh_buf, &q->br, q->has_col, q->has_uv);

    if(!success) {
        wrm_error("Menu", "addQuad()", "failed to add all vertices");
        return false;
    } 

    // add six indices (CW)
    u32 indices[] = {idx, idx + 1, idx + 3, idx, idx + 3, idx + 2};

    if(!wrm_menu_addIndices(&wrm_mesh_buf, indices, 6)) {
        wrm_error("Menu", "addQuad()", "failed to add indices");
        return false;
    }
    return true;
}

wrm_Option_Handle wrm_menu_createRenderData(wrm_Menu_Element *e)
{
    if(!e) return OPTION_NONE(Handle);

    // reset mesh buffer
    wrm_menu_resetMeshBuffer(&wrm_mesh_buf);


    // convert pixel width and height to NDC coords for model
    float width = (float)e->width * ndc_scale_w;
    float height = (float)e->height * ndc_scale_h;
    // initialize with 0.0, 0.0 as the top left corner of the quad
    vec2 tl = { 0.0f, 0.0f };

    // start with just a simple error box
    wrm_Quad q;
    wrm_menu_setQuadPositions(&q, tl, width, height);
    vec2 tl_uv = {0.0f, 0.0f};
    float uv_w = 1.0f;
    float uv_h = 1.0f;
    wrm_menu_setQuadUVs(&q, tl_uv, uv_w, uv_h);

    wrm_menu_addQuad(&wrm_mesh_buf, &q);

    wrm_Mesh_Data mesh_data = {
        .cw = true,
        .dynamic = true,
        .mode = WRM_MESH_TRIANGLE,
        .positions = wrm_mesh_buf.positions,
        .colors = NULL,
        .uvs = wrm_mesh_buf.uvs,
        .indices = wrm_mesh_buf.indices,
        .idx_cnt = wrm_mesh_buf.idx_len,
        .vtx_cnt = wrm_mesh_buf.vtx_len
    };

    wrm_Option_Handle result = wrm_render_createMesh(&mesh_data);
    if(!result.exists) {
        return result;
    }
    printf("Menu: mesh created! [%u]\n", result.val);

    wrm_Model_Data model_data = {
        .pos = {e->x, e->y, 0.0f},
        .rot = {0.0f, 0.0f, 0.0f},
        .scale = {1.0f, 1.0f, 1.0f},
        .mesh = result.val,
        .texture = 0,
        .shader = wrm_shader_defaults.texture,
        .is_visible = true,
        .is_ui = true
    };

    result = wrm_render_createModel(&model_data, 0, true);
    return result;
}

bool wrm_menu_exists(wrm_Handle h, const char *caller, const char *type, const char *reason)
{
    if(wrm_elements.is_used[h]) { return true; }

    fprintf(stderr, "ERROR: Menu: %s: %s [%u] does not exist %s\n", caller, type, h, reason);
    return false;
}

void wrm_menu_createTestElement(void)
{

    wrm_Text_Box tb = {
        .font = 0,
        .text = "Hello menu!",
        .text_len = strlen("Hello menu!"),
    };

    wrm_Option_Handle result = wrm_menu_createTextBox(&tb, -0.5f, 0.5f, 50, 70, 0);
    if(!result.exists) {
        if(wrm_menu_settings.errors) { wrm_error("Menu", "init()", "failed to create test element!"); }
        return;
    }
    if(wrm_menu_settings.verbose) { printf("Menu: successfully created test element: [%u]\n", result.val); }
}