#version 430

#define FLIP_DIAG (1<<0)
#define FLIP_X (1<<1)
#define FLIP_Y (1<<2)

in vec2 vert;

uniform uvec2 canvas;
uniform ivec4 coords;
uniform ivec2 position;
uniform uint flags;
uniform sampler2D tex;
layout(std140) uniform camera_block {
    ivec2 camera;
    float zoom;
};

out vec2 tex_coords;

void main() {
    vec2 pos = position;
    pos += vert * coords.zw - camera * zoom + (canvas * zoom) / 2;
    pos *= 2.0 / (canvas * zoom);
    pos -= 1;
    pos.y *= -1;
    gl_Position = vec4(pos, 0, 1);

    vec2 tex_axis = vec2(vert.x, 1 - vert.y);
    vec2 tex_size = textureSize(tex, 0).xy;
    if (bool(flags & uint(FLIP_X)))
        tex_axis.x = 1 - tex_axis.x;
    if (bool(flags & uint(FLIP_Y)))
        tex_axis.y = 1 - tex_axis.y;
    if (bool(flags & uint(FLIP_DIAG)))
        tex_axis = 1 - tex_axis.yx;

    tex_coords = coords.xy / tex_size + tex_axis * coords.zw / tex_size;
}
