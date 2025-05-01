#pragma once
namespace Shaders {
typedef const char asset_t[];

extern asset_t batch_render_vert;
extern asset_t batch_render_frag;
}  // namespace Shaders

namespace Textures {
struct asset_t {
    unsigned long long length;
    unsigned char data[];
};

extern asset_t main_tileset;
extern asset_t background;
extern asset_t Crystal;
}  // namespace Textures

namespace Maps {
typedef const char asset_t[];

extern asset_t test_map;
}  // namespace Maps

namespace Tilesets {
typedef const char asset_t[];

extern asset_t main_tileset;
}  // namespace Tilesets
