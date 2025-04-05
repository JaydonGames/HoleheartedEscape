#pragma once
namespace Shaders {
    typedef const char shader[];

    extern shader batch_render_vert;
    extern shader batch_render_frag;
}

namespace Textures {
    typedef const unsigned char texture[];
    typedef const unsigned long long length;

    extern texture square_png;
    extern texture tiles_png;
    extern length square_png_length;
    extern length tiles_png_length;
}

