#pragma once
#include <vector>
#include "opengl.hpp"
#include "types.hpp"

namespace Render {
    using OpenGL::Texture;

    class Camera {
    public:
        Camera();
        void set(int x, int y);
        void set(int x, int y, float zoom);
        void bind();
        OpenGL::Buffer ubo;
    };

    class Canvas {
    public:
        unsigned int x, y;
        OpenGL::Framebuffer fbo;
    };

    enum Flags {
        FlipDiag = 1 << 0,
        FlipX = 1 << 1,
        FlipY = 1 << 2,
        FlipXY = FlipX | FlipY
    };

    class TextureGroup {
    public:
        TextureGroup() = default;
        TextureGroup(TextureGroup&&) = default;
        TextureGroup(TextureGroup&);

        size_t push_back(uint8_t data[], size_t size);
        size_t push_back(Textures::asset_t& texture);
        void bind(unsigned int);
        void finalize();

    private:
        struct Texture {
            uint8_t* data;
            size_t size;
        };

        std::vector<Texture> textures;
        OpenGL::Texture tex_arr;
    };

    class BatchRenderer {
    public:
        BatchRenderer();
        void push(const Vec2& pos, const Rect& tex_coords, size_t tex, unsigned int flags = 0);
        void render(Canvas& canvas, TextureGroup& textures);
        void clear(Canvas& canvas);

    private:
        OpenGL::Program program;
        OpenGL::Uniform canvas_size;
        OpenGL::VertexArray vao;

        struct Quad {
            Vec2 pos;
            Rect tex_coords;
            unsigned int tex, flags;
        };

        std::vector<Quad> quads;
    };

}
