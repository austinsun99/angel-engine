#version 450

layout(location = 0) out vec3 frag_colour;

void main()
{
    const vec3 positions[3] = vec3[3](
            vec3(.8f, .8f, .2f),
            vec3(-.8f, .8f, 0.0f),
            vec3(0.f, -.8f, 0.0f)
        );

    const vec3 colours[3] = vec3[3](
            vec3(1.f, 0.f, 0.f),
            vec3(0.f, 1.f, 0.f),
            vec3(0.f, 0.f, 1.f)
        );

    gl_Position = vec4(positions[gl_VertexIndex], 1.0f);
    frag_colour = colours[gl_VertexIndex];
}
