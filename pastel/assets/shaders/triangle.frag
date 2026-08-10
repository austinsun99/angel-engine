#version 450

layout(location = 0) in vec3 frag_colour;
layout(location = 0) out vec4 out_frag_colour;

void main() {
    out_frag_colour = vec4(frag_colour, 1.0f);
}
