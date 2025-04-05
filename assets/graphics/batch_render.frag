#version 430
flat in uint tex_id;
in vec2 tex_xy;
out vec4 color;

uniform sampler2D tex[@];

void main(){
    color = texture(tex[tex_id], tex_xy);
}
