#include "render.h"

// shader globals/constants

const u32 WRM_SHADER_ATTRIB_POS_LOC = 0;
const u32 WRM_SHADER_ATTRIB_COL_LOC = 1;
const u32 WRM_SHADER_ATTRIB_UV_LOC = 2;

const char *WRM_SHADER_DEFAULT_COL_NAME = "default-color";
const char *WRM_SHADER_DEFAULT_TEX_NAME = "default-texture";

wrm_Default_Shaders wrm_default_shaders;

// user-visible

wrm_Option_Handle wrm_render_createShader(const char *vert_text, const char *frag_text, wrm_render_Format format)
{
    wrm_Option_Handle pool_result = wrm_Pool_getSlot(&wrm_shaders);

    if(!pool_result.exists) return pool_result;

    wrm_Shader *s = wrm_Pool_AT(wrm_shaders, wrm_Shader, pool_result.val);
    s->format = format;

    // first compile the vertex and fragment shaders individually
    wrm_Option_GLuint result = wrm_render_compileShader(vert_text, GL_VERTEX_SHADER);
    if(!result.exists) {
        wrm_render_deleteShader(result.val);
        return OPTION_NONE(Handle);
    }
    s->vert = result.val;
    
    result = wrm_render_compileShader(frag_text, GL_FRAGMENT_SHADER);
    if(!result.exists) {
        wrm_render_deleteShader(result.val);
        return OPTION_NONE(Handle);
    }
    s->frag = result.val;
    
    // then link them to form a program

    GLuint program = glCreateProgram();

    glAttachShader(program, s->vert);
    glAttachShader(program, s->frag);

    glLinkProgram(program);

    GLint success = 0;
    glGetProgramiv(program, GL_LINK_STATUS, &success);

    if(success == GL_FALSE) {
        if(wrm_render_settings.errors) {
            GLint log_len = 0;
            glGetProgramiv(program, GL_INFO_LOG_LENGTH, &log_len);

            char *log_msg = malloc(log_len * sizeof(char));
            glGetProgramInfoLog(program, log_len, NULL, log_msg);
            fprintf(stderr, "ERROR: Render: failed to link shaders, GL error: %s", log_msg);
            free(log_msg);
        }

        wrm_render_deleteShader(result.val);
        return OPTION_NONE(Handle);
    }

    glDetachShader(program, s->vert);
    glDetachShader(program, s->frag);

    s->program = program;

    if (s->format.tex) {
        glUseProgram(program);
        GLint tex_uniform = glGetUniformLocation(program, "tex");
        if (tex_uniform != -1) {
            glUniform1i(tex_uniform, 0); // Assumes all your textured shaders use GL_TEXTURE0: can later extend to use multiple textures
        }
        glUseProgram(0); // Optional: Unbind the program
    }

    return pool_result;
}

void wrm_render_printShaderData(wrm_Handle shader)
{
    if(!wrm_render_exists(shader, WRM_RENDER_RESOURCE_SHADER, "printShaderData()", "")) return;
    
    wrm_Shader *s = wrm_Pool_AT(wrm_shaders, wrm_Shader, shader);

    printf(
        "[%u]: { \n"
        "  format: { tex: %s, col: %s, per_pos: %u },\n"
        "  vert: %u, frag: %u, program: %u     }\n", 
        shader,
        s->format.tex ? "true" : "false", 
        s->format.col ? "true" : "false",
        s->format.per_pos,
        s->vert,
        s->frag,
        s->program
    );
}

// module internal 

wrm_Option_GLuint wrm_render_compileShader(const char *shader_text, GLenum type) 
{
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &shader_text, NULL);
    glCompileShader(shader);

    // error checking
    GLint success = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

    if(success == GL_FALSE) {
        if(wrm_render_settings.errors) {
            GLint log_len = 0;
            glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &log_len);

            char *log_msg = malloc(log_len * sizeof(char));
            glGetShaderInfoLog(shader, log_len, NULL, log_msg);

            fprintf(stderr, "ERROR: Render: failed to compile shader, GL error: %s\n Shader source: %s\n", log_msg, shader_text);
            free(log_msg);
        }

        return (wrm_Option_GLuint){ .exists = false };
    }

    return (wrm_Option_GLuint){ .exists = true, .val = shader };
}

wrm_Option_Handle wrm_render_getDefaultShader(wrm_Handle mesh)
{  
    const char *caller = "getDefaultShader()";
    if(!wrm_render_exists(mesh, WRM_RENDER_RESOURCE_MESH, caller, "")) {
        return OPTION_NONE(Handle);
    }
    wrm_Mesh *m = wrm_Pool_AT(wrm_meshes, wrm_Mesh, mesh);
    
    // prioritize texture over color
    if(m->uv_vbo) {
        return (wrm_Option_Handle){.exists = true, .val =  wrm_default_shaders.texture};
    }
    if(m->col_vbo) {
        return (wrm_Option_Handle){.exists = true, .val =  wrm_default_shaders.color};
    }
    
    // all other options failed
    if(wrm_render_settings.errors) wrm_error("Render", caller, "No suitable default shader for mesh [%u] found\n", mesh);
    return OPTION_NONE(Handle);
}

bool wrm_render_createDefaultShaders(const char *shaders_dir)
{
    if(!shaders_dir) return false;
    
    wrm_render_Format col_format = {
        .col = true,
        .tex = false,
        .per_pos = 3
    };

    wrm_Option_Handle result = wrm_render_loadAndCreateShader(shaders_dir, WRM_SHADER_DEFAULT_COL_NAME, col_format);
    if(!result.exists) {
        return false;
    }
    wrm_default_shaders.color = result.val;


    wrm_render_Format tex_format = {
        .col = false,
        .tex = true,
        .per_pos = 3
    };

    result = wrm_render_loadAndCreateShader(shaders_dir, WRM_SHADER_DEFAULT_TEX_NAME, tex_format);
    if(!result.exists) {
        return false;
    }
    wrm_default_shaders.texture = result.val;

    return true;
}

wrm_Option_Handle wrm_render_loadAndCreateShader(const char *dir, const char *name, wrm_render_Format format)
{
    size_t dir_len = strlen(dir);
    size_t name_len = strlen(name);

    // include slash and .vert
    size_t len = dir_len + 1 + name_len + 5;
    // include null terminator
    char *vert_path = malloc(len + 1);
    char *frag_path = malloc(len + 1);

    sprintf(vert_path, "%s/%s.vert", dir, name);
    sprintf(frag_path, "%s/%s.frag", dir, name);

    char *vert = wrm_readFile(vert_path);
    char *frag = wrm_readFile(frag_path);

    wrm_Option_Handle result; 
    result = wrm_render_createShader(vert, frag, format);
    free(vert);
    free(frag);
    free(vert_path);
    free(frag_path);
    
    return result;
}

void wrm_render_deleteShader(wrm_Handle shader)
{
    if(!wrm_render_exists(shader, WRM_RENDER_RESOURCE_SHADER, "deleteShader()", "")) return;

    wrm_Shader *s = wrm_Pool_AT(wrm_shaders, wrm_Shader, shader);

    glDeleteShader(s->vert);
    glDeleteShader(s->frag);
    glDeleteProgram(s->program);

    wrm_Pool_freeSlot(&wrm_shaders, shader);
}



