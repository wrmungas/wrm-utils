#version 330 core 

layout (location = 0) in vec2 v_pos; // positions are location 0
layout (location = 2) in vec2 v_uv; // uvs are location 2

out vec2 uv; // specify a uv for the fragment shader

void main()
{
    gl_Position = vec4(v_pos, 1.0, 1.0);
    uv = v_uv;
}