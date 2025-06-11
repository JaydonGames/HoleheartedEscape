#version 430

in vec2 vert;
out vec2 tex_coords;
out vec2 world_coords;

layout(std140) uniform camera_block {
    ivec2 camera;
    float zoom;
};

layout(std140) uniform canvas_block {
    uvec2 canvas;
};

void main() {
    gl_Position = vec4(vert * 2 - 1, 0, 1);
    tex_coords = vert;
    world_coords = camera - canvas / 2 + zoom * canvas * vec2(vert.x, 1 - vert.y);
}
