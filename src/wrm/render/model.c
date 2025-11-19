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
        wrm_render_setModelShader(result.val, shader) && 
        wrm_render_setModelMesh(result.val, data->mesh) && 
        wrm_render_setModelTexture(result.val, data->texture) &&
        wrm_render_setModelTransform(result.val, data->pos, data->rot, data->scale) &&
        (!parent || wrm_render_addChild(parent, result.val));

    if(!update_success) {
        wrm_Pool_freeSlot(&wrm_models, result.val);
        return OPTION_NONE(Handle);
    }

    wrm_Model* model = wrm_Pool_at(&wrm_models, result.val);

    // explicitly zero-initialize tree node
    model->tree_node = (wrm_Tree_Node){ 0 };

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
    wrm_Model *data = wrm_Pool_at(&wrm_models, model);
    if(!data) { return false; }

    if(pos) wrm_vec3_copy(pos, data->pos);
    if(rot) wrm_vec3_copy(rot, data->rot);
    if(scale) wrm_vec3_copy(scale, data->scale);

    return true;
}

bool wrm_render_addModelTransform(wrm_Handle model, const vec3 pos, const vec3 rot, const vec3 scale)
{
    wrm_Model *data = wrm_Pool_at(&wrm_models, model);
    if(!data) { return false; }

    if(pos) wrm_vec3_add(pos, data->pos);
    if(rot) wrm_vec3_add(rot, data->rot);
    if(scale) wrm_vec3_add(scale, data->scale);

    return true;
}

bool wrm_render_setModelMesh(wrm_Handle model, wrm_Handle mesh) 
{
    wrm_Model *m = wrm_Pool_at(&wrm_models, model);
    wrm_Mesh *msh = wrm_Pool_at(&wrm_meshes, mesh);
    if(!m || !msh) { return false; }

    m->mesh = mesh;
    return true;
}

bool wrm_render_setModelTexture(wrm_Handle model, wrm_Handle texture)
{
    wrm_Model *m = wrm_Pool_at(&wrm_models, model);
    wrm_Texture *t = wrm_Pool_at(&wrm_textures, texture);

    if(!m || !t) { return false; }

    m->texture = texture;
    return true;
}

bool wrm_render_setModelShader(wrm_Handle model, wrm_Handle shader)
{
    wrm_Model *mod = wrm_Pool_at(&wrm_models, model);
    wrm_Shader *s = wrm_Pool_at(&wrm_shaders, shader);
    if(!mod || !s) { return false; }

    wrm_Mesh *mesh = wrm_Pool_at(&wrm_meshes, mod->mesh);
    if(!mesh) { return false; }

    // ensure the shader and mesh are compatible
    wrm_render_Format sf = s->format;
    wrm_render_Format mf = mesh->format;
    if( (sf.col && !mf.col) || (sf.per_pos && !mf.per_pos) || (sf.tex && !mf.tex)) {
        if(wrm_render_settings.errors) wrm_error("Render", "setModelShader()", "Mesh [%u] does not meet shader [%u] data requirements", mod->mesh, shader);
        return false;
    }

    mod->shader = shader;
    return true;
}

bool wrm_render_addChild(wrm_Handle parent, wrm_Handle child)
{
    return wrm_Tree_addChild(&wrm_model_tree, parent, child);
}

bool wrm_render_removeChild(wrm_Handle parent, wrm_Handle child)
{
    return wrm_Tree_removeChild(&wrm_model_tree, parent, child);
}

void wrm_render_debugModel(wrm_Handle model)
{
    if(!wrm_render_exists(model, WRM_RENDER_RESOURCE_MODEL, "printModelData()", "")) return;

    wrm_Model *m = wrm_Pool_at(&wrm_models, model);
    printf(
        "[%u]:\n { shader: %u, texture: %u, mesh: %u, is_visible: %s, show_children: %s, "
        "pos: < %.2f %.2f %.2f >, rot: < %.2f %.2f %.2f >, scale: < %.2f %.2f %.2f >, tree_node:"
        , 
        model, 
        m->shader,
        m->texture,
        m->mesh,
        m->is_visible ? "true" : "false",
        m->show_children ? "true" : "false",
        m->pos[0], m->pos[1], m->pos[2],
        m->rot[0], m->rot[1], m->rot[2],
        m->scale[0], m->scale[1], m->scale[2]
    );

    wrm_Tree_debugNode(&m->tree_node, &wrm_model_tree);
    printf("}\n");
}

void wrm_render_deleteModel(wrm_Handle model)
{
    wrm_Model_delete(wrm_Pool_at(&wrm_models, model));
    wrm_Pool_freeSlot(&wrm_models, model);
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

void wrm_Model_delete(void *model)
{
    if(!model) return;
    wrm_Model *m = model;

    // if any of these are non-NULL they will be cleaned up
    wrm_Shader_delete(wrm_Pool_at(&wrm_shaders, m->shader));
    wrm_Mesh_delete(wrm_Pool_at(&wrm_meshes, m->mesh));
    wrm_Texture_delete(wrm_Pool_at(&wrm_textures, m->texture));
}