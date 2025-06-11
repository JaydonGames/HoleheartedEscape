#version 430

in vec2 tex_coords;

uniform sampler2D tex;

layout(location = 0) out vec4 color;

void main(){
    color = texture(tex, tex_coords);
}
