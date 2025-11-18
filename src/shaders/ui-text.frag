// file: ui-text.frag
#version 330 core

in vec2 uv;
out vec4 color;

uniform sampler2D tex;
uniform vec3 text_col;

void main()
{    
    vec4 sampled = vec4(1.0, 1.0, 1.0, texture(tex, uv).r);
    color = vec4(text_col, 1.0) * sampled;
}  

