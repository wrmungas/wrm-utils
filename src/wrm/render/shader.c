#include "render.h"

// shader globals/constants

const u32 WRM_SHADER_ATTRIB_POS_LOC = 0;
const u32 WRM_SHADER_ATTRIB_COL_LOC = 1;
const u32 WRM_SHADER_ATTRIB_UV_LOC = 2;

const char * WRM_SHADER_DEFAULT_COL_V_TEXT = {
"#version 330 core\n"
"layout (location = 0) in vec3 v_pos;\n" // positions are location 0
"layout (location = 1) in vec4 v_col;\n" // colors are location 1
"uniform mat4 model;\n"
"uniform mat4 persp;\n"
"uniform mat4 view;\n"
"out vec4 col;\n" // specify a color output to the fragment shader
"void main()\n"
"{\n"
"    gl_Position = persp * view * model * vec4(v_pos, 1.0);\n"
"    col = v_col;\n"
"}\n"
};
const char * WRM_SHADER_DEFAULT_COL_F_TEXT = {
"#version 330 core\n"
"in vec4 col;\n"
"out vec4 FragColor;\n"
"void main()\n"
"{\n"
"    FragColor = col;\n"
"}\n"
};

// texture
const char * WRM_SHADER_DEFAULT_TEX_V_TEXT = {
"#version 330 core\n"
"layout (location = 0) in vec3 v_pos;\n" // positions are location 0
"layout (location = 2) in vec2 v_uv;\n"  // uvs are location 2
"uniform mat4 model;\n"
"uniform mat4 persp;\n"
"uniform mat4 view;\n"
"out vec2 uv;\n" // specify a uv for the fragment shader
"void main()\n"
"{\n"
"    gl_Position = persp * view * model * vec4(v_pos, 1.0);\n" 
"    uv = v_uv;\n"
"}\n"
};
const char * WRM_SHADER_DEFAULT_TEX_F_TEXT = {
"#version 330 core\n"
"in vec2 uv;\n"
"uniform sampler2D tex;\n"
"out vec4 FragColor;\n"
"void main()\n"
"{\n"
"    FragColor = texture(tex, uv);\n"
"}\n"
};

// both
const char * WRM_SHADER_DEFAULT_BOTH_V_TEXT = {
"#version 330 core\n"
"layout (location = 0) in vec3 v_pos;\n" // positions are location 0
"layout (location = 1) in vec4 v_col;\n"
"layout (location = 2) in vec2 v_uv;\n"  // uvs are location 2
"uniform mat4 model;\n"
"uniform mat4 persp;\n"
"uniform mat4 view;\n"
"out vec4 col;\n" // specify a color for the fragment shader
"out vec2 uv;\n" // specify a uv for the fragment shader\n"
"void main()\n"
"{\n"
"    gl_Position = persp * view * model * vec4(v_pos, 1.0);\n"
"    col = v_col;\n" 
"    uv = v_uv;\n"
"}\n"
};
const char * WRM_SHADER_DEFAULT_BOTH_F_TEXT = {
"#version 330 core\n"
"in vec4 col;\n"
"in vec2 uv;\n"
"uniform sampler2D tex;\n"
"out vec4 FragColor;\n"
"void main()\n"
"{\n"
"    FragColor = texture(tex, uv) * col;\n"
"}\n"
};

wrm_Shader_Defaults wrm_shader_defaults;

// user-visible

wrm_Option_Handle wrm_render_createShader(const char *vert_text, const char *frag_text, bool needs_col, bool needs_tex)
{
    wrm_Option_Handle pool_result = wrm_Pool_getSlot(&wrm_shaders);

    if(!pool_result.exists) return pool_result;

    wrm_Shader s;
    s.needs_col = needs_col;
    s.needs_tex = needs_tex;

    // first compile the vertex and fragment shaders individually
    wrm_Option_GLuint result = wrm_render_compileShader(vert_text, GL_VERTEX_SHADER);
    if(!result.exists) {
        wrm_Pool_freeSlot(&wrm_shaders, pool_result.val);
        return (wrm_Option_Handle){.exists = false };
    }
    s.vert = result.val;
    
    result = wrm_render_compileShader(frag_text, GL_FRAGMENT_SHADER);
    if(!result.exists) {
        wrm_Pool_freeSlot(&wrm_shaders, pool_result.val);
        glDeleteShader(s.vert);
        return (wrm_Option_Handle){.exists = false };
    }
    s.frag = result.val;
    
    // then link them to form a program

    GLuint program = glCreateProgram();

    glAttachShader(program, s.vert);
    glAttachShader(program, s.frag);

    glLinkProgram(program);

    GLint success = 0;
    glGetProgramiv(program, GL_LINK_STATUS, &success);

    if(success == GL_FALSE) {
        if(wrm_render_settings.errors) {
            GLint log_len = 0;
            glGetProgramiv(program, GL_INFO_LOG_LENGTH, &log_len);

            char *log_msg = malloc(log_len * sizeof(char));
            glGetProgramInfoLog(program, log_len, NULL, log_msg);
            fprintf(stderr, "ERROR: Render: failed to link shaders, GL error: %s\n", log_msg);
            free(log_msg);
        }

        glDeleteProgram(program);
        glDeleteShader(s.vert);
        glDeleteShader(s.frag);

        wrm_Pool_freeSlot(&wrm_shaders, pool_result.val);
        return (wrm_Option_Handle){ .exists = false };
    }

    glDetachShader(program, s.vert);
    glDetachShader(program, s.frag);

    s.program = program;

    if (s.needs_tex) {
        glUseProgram(program);
        GLint tex_uniform = glGetUniformLocation(program, "tex");
        if (tex_uniform != -1) {
            glUniform1i(tex_uniform, 0); // Assumes all your textured shaders use GL_TEXTURE0: can later extend to use multiple textures
        }
        glUseProgram(0); // Optional: Unbind the program
    }

    ((wrm_Shader*)wrm_shaders.data)[pool_result.val] = s;
    return pool_result;
}

void wrm_render_printShaderData(wrm_Handle shader)
{
    if(!wrm_render_exists(shader, WRM_RENDER_RESOURCE_SHADER, "printShaderData()", "")) return;
    
    wrm_Shader *s = wrm_Pool_dataAs(wrm_shaders, wrm_Shader);

    u32 i = shader;
    printf(
        "[%u]: { needs_col: %s, needs_tex: %s, vert: %u, frag: %u, program: %u }\n", 
        i,
        s[i].needs_col ? "true" : "false", 
        s[i].needs_tex ? "true" : "false",
        s[i].vert,
        s[i].frag,
        s[i].program
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

        glDeleteShader(shader);

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
    wrm_Mesh *m = wrm_Pool_dataAt(wrm_meshes, wrm_Mesh, mesh);

    if(m->col_vbo && m->uv_vbo) {
        return (wrm_Option_Handle){.exists = true, .val =  wrm_shader_defaults.both};
    }
    
    if(m->col_vbo) {
        return (wrm_Option_Handle){.exists = true, .val =  wrm_shader_defaults.color};
    }

    if(m->uv_vbo) {
        return (wrm_Option_Handle){.exists = true, .val =  wrm_shader_defaults.texture};
    }
    
    // all other options failed
    if(wrm_render_settings.errors) wrm_error("Render", caller, "No suitable default shader for mesh [%u] found\n", mesh);
    return OPTION_NONE(Handle);
}

void wrm_render_createDefaultShaders(void)
{
    wrm_Option_Handle result; 
    result = wrm_render_createShader(WRM_SHADER_DEFAULT_COL_V_TEXT, WRM_SHADER_DEFAULT_COL_F_TEXT, true, false);
    if(!result.exists) {
        if(wrm_render_settings.errors) { fprintf(stderr, "ERROR: Render: failed to create default color shader\n"); }
    }
    wrm_shader_defaults.color = result.val;

    result = wrm_render_createShader(WRM_SHADER_DEFAULT_TEX_V_TEXT, WRM_SHADER_DEFAULT_TEX_F_TEXT, false, true);
    if(!result.exists) {
        if(wrm_render_settings.errors) { fprintf(stderr, "ERROR: Render: failed to create default texture shader\n"); }
    }
    wrm_shader_defaults.texture = result.val;

    result = wrm_render_createShader(WRM_SHADER_DEFAULT_BOTH_V_TEXT, WRM_SHADER_DEFAULT_BOTH_F_TEXT, true, true);
    if(!result.exists) {
        if(wrm_render_settings.errors) { fprintf(stderr, "ERROR: Render: failed to create default color + texture shader\n"); }
    }
    wrm_shader_defaults.both  = result.val;
}
