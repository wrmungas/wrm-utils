// file: ui-text.frag
#version 330 core

in vec2 uv;

uniform sampler2D tex;
uniform vec3 text_col;

out vec4 f_col;

void main()
{    
    vec4 sampled = vec4(1.0, 1.0, 1.0, texture(tex, uv).r);
    f_col = vec4(text_col, 1.0) * sampled;
}  

