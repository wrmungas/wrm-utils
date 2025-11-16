#version 330 core 
layout (location = 0) in vec3 v_pos; // positions are location 0
layout (location = 2) in vec2 v_uv; // uvs are location 2
uniform mat4 model;
uniform mat4 persp;
uniform mat4 view;
out vec2 uv; // specify a uv for the fragment shader
void main()
{
    gl_Position = persp * view * model * vec4(v_pos, 1.0);
    uv = v_uv;
}