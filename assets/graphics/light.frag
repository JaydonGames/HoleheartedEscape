#version 430

in vec2 tex_coords;
in vec2 world_coords;
layout(location = 0) out vec4 color_out;

uniform sampler2D tex;
layout(std140) uniform camera_block {
    ivec2 camera;
    float zoom;
};

layout(std140) uniform canvas_block {
    uvec2 canvas;
};

layout(std140) uniform light_block {
    ivec2 pos;
    uint radius;
    float intensity;
    vec3 color;
} light;

void main() {
    float shadow = texture(tex, tex_coords).r;
    float dist = distance(world_coords, light.pos);
    if (dist >= light.radius)
        discard;
    color_out = vec4(light.color, (1 - dist/light.radius) * light.intensity * (1 - shadow));
}
