#include "render.h"


// user-visible

wrm_Option_Handle wrm_render_createModel(const wrm_Model_Data *data, wrm_Handle parent, bool use_default_shader)
{
    wrm_Option_Handle result = wrm_Pool_getSlot(&wrm_models);

    const char *caller = "createModel()";
     
    if(!result.exists) {
        if(wrm_render_settings.errors) { wrm_error("Render", caller, "failed to create model\n"); }
        return result;
    }

    bool has_resources = 
        (!parent || wrm_render_exists(parent, WRM_RENDER_RESOURCE_MODEL, caller, "(parent)")) &&
        wrm_render_exists(data->mesh, WRM_RENDER_RESOURCE_MESH, caller, "") &&
        wrm_render_exists(data->texture, WRM_RENDER_RESOURCE_TEXTURE, caller, "");
    
    if(!has_resources) {
        wrm_Pool_freeSlot(&wrm_models, result.val);
        return OPTION_NONE(Handle);
    }
    
    wrm_Handle shader = data->shader;
    if(use_default_shader) {
        wrm_Option_Handle default_shader = wrm_render_getDefaultShader(data->mesh);
        if(!default_shader.exists) {
            wrm_Pool_freeSlot(&wrm_models, result.val);
            return OPTION_NONE(Handle);
        }
    }

    bool update_success = 
        wrm_render_updateModelShader(result.val, shader) && 
        wrm_render_updateModelMesh(result.val, data->mesh) && 
        wrm_render_updateModelTexture(result.val, data->texture) &&
        wrm_render_updateModelTransform(result.val, data->pos, data->rot, data->scale) &&
        (!parent || wrm_render_addChild(parent, result.val));

    if(!update_success) {
        wrm_Pool_freeSlot(&wrm_models, result.val);
        return OPTION_NONE(Handle);
    }

    wrm_Model* model = wrm_Pool_dataAt(wrm_models, wrm_Model, result.val);

    // explicitly zero-initialize children;
    model->child_count = 0;
    for(u8 i = 0; i < WRM_MODEL_CHILD_LIMIT; i++) {
        model->children[i] = 0;
    }
    model->is_ui = data->is_ui;
    model->is_visible = data->is_visible;
    model->show_children = true;

    if(model->is_ui) { wrm_ui_count++; }

    return result;
}

bool wrm_render_getModel(wrm_Handle model, wrm_Model *dest)
{
    if(!wrm_render_exists(model, WRM_RENDER_RESOURCE_MODEL, "getModel()", "")) {
        return false;
    }
    *dest = ((wrm_Model*)wrm_models.data)[model];
    return true;
}

bool wrm_render_setModelTransform(wrm_Handle model, const vec3 pos, const vec3 rot, const vec3 scale)
{
    if(!wrm_render_exists(model, WRM_RENDER_RESOURCE_MODEL, "updateModelTransform()", "")) {
        return false;
    }
    wrm_Model *data = wrm_Pool_dataAt(wrm_models, wrm_Model, model);

    if(pos) wrm_vec3_copy(pos, data->pos);
    if(rot) wrm_vec3_copy(rot, data->rot);
    if(scale) wrm_vec3_copy(scale, data->scale);

    return true;
}

bool wrm_render_addModelTransform(wrm_Handle model, const vec3 pos, const vec3 rot, const vec3 scale)
{
    if(!wrm_render_exists(model, WRM_RENDER_RESOURCE_MODEL, "updateModelTransform()", "")) {
        return false;
    }
    wrm_Model *data = wrm_Pool_dataAt(wrm_models, wrm_Model, model);

    if(pos) wrm_vec3_add(pos, data->pos);
    if(rot) wrm_vec3_add(rot, data->rot);
    if(scale) wrm_vec3_add(scale, data->scale);

    return true;
}

bool wrm_render_setModelMesh(wrm_Handle model, wrm_Handle mesh) 
{
    const char *caller = "updateModelMesh()";
    if(wrm_render_exists(model, WRM_RENDER_RESOURCE_MODEL, caller, "(model)") && wrm_render_exists(mesh, WRM_RENDER_RESOURCE_MESH, caller, "(mesh)")) {
        wrm_Pool_dataAt(wrm_models, wrm_Model, model)->mesh = mesh;
        return true;
    }
    return false;
}

bool wrm_render_setModelTexture(wrm_Handle model, wrm_Handle texture)
{
    const char *caller = "updateModelTexture()";
    if(wrm_render_exists(model, WRM_RENDER_RESOURCE_MODEL, caller, "(model)") && wrm_render_exists(texture, WRM_RENDER_RESOURCE_TEXTURE, caller, "(texture)")) {
        wrm_Pool_dataAt(wrm_models, wrm_Model, model)->texture = texture;
        return true;
    }
    return false;
}

bool wrm_render_setModelShader(wrm_Handle model, wrm_Handle shader)
{
    const char *caller = "updateModelShader()";
    if(!wrm_render_exists(model, WRM_RENDER_RESOURCE_MODEL, caller, "") && wrm_render_exists(shader, WRM_RENDER_RESOURCE_SHADER, caller, "")) {
        return false;
    }

    wrm_Model *mod = wrm_Pool_dataAt(wrm_models, wrm_Model, model);
    wrm_Shader *sh = wrm_Pool_dataAt(wrm_shaders, wrm_Shader, shader);
    wrm_Mesh *mesh = wrm_Pool_dataAt(wrm_meshes, wrm_Mesh, mod->mesh);

    // ensure the shader and mesh are compatible
    if( (sh->needs_col && !mesh->col_vbo) || (sh->needs_tex && !mesh->uv_vbo)) {
        if(wrm_render_settings.errors) wrm_error("Render", caller, "Mesh [%u] does not meet shader [%u] data requirements\n", mod->mesh, shader);
        return false;
    }

    mod->shader = shader;
    return true;
}

bool wrm_render_updateModel(wrm_Handle model, const wrm_Model *data)
{
    if(!wrm_render_exists(model, WRM_RENDER_RESOURCE_MODEL, "updateModel()", "")) {
        return false;
    }

    wrm_render_updateModelTransform(model, data->pos, data->rot, data->scale);
    wrm_render_updateModelMesh(model, data->mesh);
    wrm_render_updateModelTexture(model, data->texture);
    wrm_render_updateModelShader(model, data->shader);

    return true;
}

bool wrm_render_addChild(wrm_Handle parent, wrm_Handle child)
{
    char * caller = "addChildModel()";
    wrm_render_Resource_Type type = WRM_RENDER_RESOURCE_MODEL;
    if(!(wrm_render_exists(parent, type, caller, "(parent)") && wrm_render_exists(child, type, caller, "(child)"))) {
        return false;
    }

    wrm_Model *m = wrm_Pool_dataAs(wrm_models, wrm_Model);

    if(m[parent].child_count == WRM_MODEL_CHILD_LIMIT) {
        if(wrm_render_settings.errors) { fprintf(stderr, "ERROR: Render: cannot add another child to model [%u] (limit reached!)\n", parent); }
        return false;
    }

    if(m[child].parent) {
        if(wrm_render_settings.errors) { fprintf(stderr, "ERROR: Render: cannot add parent to model [%u] (already has parent!)\n", child); }
        return false;
    }

    for(u8 i = 0; i < m[parent].child_count; i++) {
        if(m[parent].children[i] == child) {
            if(wrm_render_settings.errors) { fprintf(stderr, "ERROR: Render: cannot add child [%u] to model [%u] (already added!)\n", child, parent); }
            return false;
        }
    }

    if(parent) m[parent].children[m[parent].child_count++] = child;
    m[child].parent = parent;
    return true;
}

bool wrm_render_removeChild(wrm_Handle parent, wrm_Handle child)
{
    char * caller = "removeChildModel()";
    wrm_render_Resource_Type type = WRM_RENDER_RESOURCE_MODEL;
    if(!(wrm_render_exists(parent, type, caller, "(parent)") && wrm_render_exists(child, type, caller, "(child)"))) {
        return false;
    }

    wrm_Model *m = wrm_Pool_dataAs(wrm_models, wrm_Model);

    if(m[parent].child_count == 0) {
        if(wrm_render_settings.errors) { fprintf(stderr, "ERROR: Render: cannot remove child from model [%u] (has no children!)\n", parent); }
        return false;
    }

    if(!m[child].parent) {
        if(wrm_render_settings.errors) { fprintf(stderr, "ERROR: Render: cannot remove parent from model [%u] (has no parent!)\n", child); }
        return false;
    }

    if(m[child].parent != parent) {
        if(wrm_render_settings.errors) { fprintf(stderr, "ERROR: Render: cannot remove parent [%u] from model [%u] (has a different parent!)\n", parent, child); }
        return false;
    }   
    
    // find the child
    u8 idx;
    for(idx = 0; idx < m[parent].child_count; idx++) {
        if(m[parent].children[idx] == child) break;
    }
    if(idx == WRM_MODEL_CHILD_LIMIT) {
        if(wrm_render_settings.errors) { fprintf(stderr, "ERROR: Render: cannot remove child [%u] from model [%u] (already removed!)\n", child, parent); }
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

void wrm_render_printModelData(wrm_Handle model)
{
    if(!wrm_render_exists(model, WRM_RENDER_RESOURCE_MODEL, "printModelData()", "")) return;

    wrm_Model m = wrm_Pool_dataAs(wrm_models, wrm_Model)[model];
    printf(
        "[%u]: { \n"
        "   shader: %u, texture: %u, mesh: %u, is_ui: %s, is_visible: %s,\n"
        "   pos: < %.2f %.2f %.2f >, rot: < %.2f %.2f %.2f >, scale: < %.2f %.2f %.2f >\n"
        "   parent: %u, child_count: %u, show_children: %s,\n"
        "   children: [ "
        , 
        model, 
        m.shader,
        m.texture,
        m.mesh,
        m.is_ui ? "true" : "false",
        m.is_visible ? "true" : "false",
        m.pos[0], m.pos[1], m.pos[2],
        m.rot[0], m.rot[1], m.rot[2],
        m.scale[0], m.scale[1], m.scale[2],
        m.parent,
        m.child_count,
        m.show_children ? "true" : "false"
    );
    for(u8 j = 0; j < WRM_MODEL_CHILD_LIMIT; j++) {
        printf("%u, ", m.children[j]);
    }
    printf("]   }\n");
}

// module internal

wrm_Option_Handle wrm_render_createTestTriangle(void)
{
    const char *caller = "createTestModel()";
    wrm_Option_Handle mesh = wrm_render_createMesh(&default_meshes_colored_triangle);
    if(!mesh.exists) {
        if(wrm_render_settings.errors) wrm_error("Render", caller, "failed to create test triangle mesh");
        return mesh;
    }
    if(wrm_render_settings.verbose) printf("Render: Created test triangle mesh (handle=%d)\n", mesh.val);

    wrm_Model_Data model_data = {
        .pos = {0.0f, 0.0f, 0.0f},
        .rot = { [WRM_PITCH] = 0.0f, [WRM_YAW] = 180.0f, [WRM_ROLL] = 0.0f},
        .scale = {1.0f, 1.0f, 1.0f},
        .mesh = mesh.val,
        .texture = 0,
        .shader = wrm_default_shaders.color,
        .is_visible = wrm_render_settings.test,
        .is_ui = false
    };
    wrm_Option_Handle model = wrm_render_createModel(&model_data, 0, false);
    if(!model.exists) {
        if(wrm_render_settings.errors) wrm_error("Render", caller, "failed to create test triangle model");
        return model;
    }
    if(wrm_render_settings.verbose) printf("Render: Created test triangle model (handle=%d)\n", model.val);
    return model;
}

wrm_Option_Handle wrm_render_createTestCube(void)
{
    const char *caller = "createTestModel()";
    wrm_Option_Handle mesh = wrm_render_createMesh(&default_meshes_textured_cube);
    if(!mesh.exists) {
        if(wrm_render_settings.errors) wrm_error("Render", caller, "failed to create test triangle mesh");
        return mesh;
    }
    if(wrm_render_settings.verbose) printf("Render: Created test cube mesh (handle=%d)\n", mesh.val);

    wrm_Model_Data model_data = {
        .pos = {0.0f, 0.0f, 0.0f},
        .rot = {0.0f, 0.0f, 0.0f},
        .scale = {1.0f, 1.0f, 1.0f},
        .mesh = mesh.val,
        .texture = 0,
        .shader = wrm_default_shaders.texture,
        .is_visible = wrm_render_settings.test,
        .is_ui = false
    };
    wrm_Option_Handle model = wrm_render_createModel(&model_data, 0, false);
    if(!model.exists) {
        if(wrm_render_settings.errors) wrm_error("Render", caller, "failed to create test triangle model");
        return model;
    }
    if(wrm_render_settings.verbose) printf("Render: Created test cube model (handle=%d)\n", model.val);
    return model;
}