#version 410

in vec2 tex_coords;
flat in uint tex_id;

layout(location = 0) out vec4 color;

uniform sampler2D tex[16];

void main(){
    color = texture(tex[tex_id], tex_coords);
}
