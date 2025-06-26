#version 430

layout(location = 0) in vec2 pos_in;
layout(location = 1) in ivec2 tex_xy_in;
layout(location = 2) in ivec2 tex_wh_in;
layout(location = 3) in uint tex_in;
layout(location = 4) in uint flags_in;
layout(location = 5) in float rotation_in;

out ivec2 vert_tex_xy;
out ivec2 vert_tex_wh;
out uint vert_tex;
out uint vert_flags;
out float vert_rotation;

void main() {
    gl_Position = vec4(pos_in, 0, 1);
    vert_tex_xy = tex_xy_in;
    vert_tex_wh = tex_wh_in;
    vert_tex = tex_in;
    vert_flags = flags_in;
    vert_rotation = rotation_in;
}
