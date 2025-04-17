#pragma once
namespace Shaders {
    typedef const char shader_t[];

    extern shader_t batch_render_vert;
    extern shader_t batch_render_frag;
}

namespace Textures {
    struct texture_t {
        unsigned long long length;
        unsigned char data[];
    };

    extern texture_t ground_tileset;
    extern texture_t walls_tileset;
}

namespace Maps {
    typedef const char map_t[];

    extern map_t map;
}

namespace Tilesets {
    typedef const char tileset_t[];

    extern tileset_t ground_tileset;
    extern tileset_t walls_tileset;
}
