#version 430

in vec2 tex_coords;
flat in uint tex_id;

layout(location = 0) out vec4 color;

uniform sampler2DArray tex_array;

void main(){
    color = texture(tex_array, vec3(tex_coords, tex_id));
}
