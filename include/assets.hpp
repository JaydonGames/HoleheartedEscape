#pragma once
namespace Shaders {
    typedef const char asset_t[];

    extern asset_t batch_render_vert;
    extern asset_t batch_render_frag;
    extern asset_t testing_comp;
}

namespace Textures {
    struct asset_t {
        unsigned long long length;
        unsigned char data[];
    };

    extern asset_t main_tileset;
    extern asset_t background;
    extern asset_t Crystal;
    extern asset_t objects;
}

namespace Maps {
    typedef const char asset_t[];

    extern asset_t test_map;
}

namespace Tilesets {
    typedef const char asset_t[];

    extern asset_t main_tileset;
}
