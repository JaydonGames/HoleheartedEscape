#version 410
layout(location = 0) in vec2 vert;
flat out uint tex_id;
out vec2 tex_xy;

uniform uvec2 screen;
uniform uvec2 tex_wh[@];
uniform ivec2 camera;

#define FLIP_DIAG (1<<0)
#define FLIP_X (1<<1)
#define FLIP_Y (1<<2)

struct Quad {
    ivec2 pos;
    ivec2 scale;
    ivec2 tex_xy;
    ivec2 tex_wh;
    uint tex;
    uint flags;
};

layout (std140) uniform quad_block {
    Quad quads[512];
};

void main() {
    Quad quad = quads[gl_VertexID/4];

    vec2 pos = quad.pos + vert*quad.scale - camera + screen/2;
    pos *= 2.0/screen;
    pos -= 1;
    pos.y *= -1;

    vec2 tex_axis = vert;
    if (bool(quad.flags & FLIP_X))
        tex_axis.x = 1 - tex_axis.x;
    if (bool(quad.flags & FLIP_Y))
        tex_axis.y = 1 - tex_axis.y;
    if (bool(quad.flags & FLIP_DIAG))
        tex_axis = tex_axis.yx;

    gl_Position = vec4(pos, 0, 1);
    tex_id = quad.tex;
    tex_xy = vec2(quad.tex_xy)/tex_wh[tex_id] + tex_axis*vec2(quad.tex_wh)/tex_wh[tex_id];
}
