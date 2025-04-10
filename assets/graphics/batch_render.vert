#version 410
layout(location = 0) in vec2 vert;
flat out uint tex_id;
out vec2 tex_xy;

uniform uvec2 screen;
uniform uvec2 tex_wh[@];

struct Quad {
    uvec2 pos;
    uvec2 scale;
    uvec2 tex_xy;
    uvec2 tex_wh;
    uint tex;
};

layout (std140) uniform quad_block {
    Quad quads[500];
};

void main() {
    Quad quad = quads[gl_VertexID/4];

    vec2 pos = quad.pos + vert*quad.scale;
    pos *= 2.0/screen;
    pos -= 1;
    pos.y *= -1;

    gl_Position = vec4(pos, 0, 1);
    tex_id = quad.tex;
    tex_xy = vec2(quad.tex_xy)/tex_wh[tex_id] + vert*vec2(quad.tex_wh)/tex_wh[tex_id];
}
