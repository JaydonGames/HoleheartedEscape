#pragma once
namespace Shaders {
typedef const char shader_t[];

extern shader_t batch_render_vert;
extern shader_t batch_render_frag;
}  // namespace Shaders

namespace Textures {
struct texture_t {
    unsigned long long length;
    unsigned char data[];
};

extern texture_t main_tileset;
extern texture_t background;
extern texture_t Crystal;
}  // namespace Textures

namespace Maps {
typedef const char map_t[];

extern map_t test_map;
}  // namespace Maps

namespace Tilesets {
typedef const char tileset_t[];

extern tileset_t main_tileset;
}  // namespace Tilesets
