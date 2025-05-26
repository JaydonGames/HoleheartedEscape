#pragma once
namespace Shaders {
    typedef const char asset_t[];

    extern asset_t simple_vert;
    extern asset_t simple_frag;

    extern asset_t batch_vert;
    extern asset_t batch_tcs;
    extern asset_t batch_tes;
    extern asset_t batch_frag;

    extern asset_t shadow_tcs;
    extern asset_t shadow_tes;
    extern asset_t shadow_frag;

    extern asset_t light_vert;
    extern asset_t light_frag;
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
