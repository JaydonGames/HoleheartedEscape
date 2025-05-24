#pragma once
#include <unordered_map>
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

    class BatchRenderer {
    public:
        BatchRenderer();
        void push(const Vec2& pos, const Rect& tex_coords, Texture& tex, unsigned int flags = 0);
        void render(Canvas& canvas);
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
        std::vector<Texture*> textures;
    };

}
