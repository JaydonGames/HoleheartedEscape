#pragma once
#include <vector>
#include "opengl.hpp"
#include "types.hpp"

namespace Render {
    using OpenGL::Texture;

    class Camera {
    public:
        Camera();
        Camera(const Camera&);
        Camera& operator=(const Camera&);
        void set(int x, int y, float zoom);
        void use();

        OpenGL::Buffer ubo;

    private:
        struct Internal;
    };

    class Canvas {
    public:
        unsigned int x, y;
        OpenGL::Framebuffer fbo;

        Canvas() {}
        Canvas(unsigned int x, unsigned int y)
            : x(x),
              y(y) {}

    private:
        OpenGL::Framebuffer internal_fbo;
        OpenGL::Texture internal_color;
        OpenGL::Renderbuffer internal_rbo;
        friend class BatchRenderer;
    };

    enum Flags {
        FlipDiag = 1 << 0,
        FlipX = 1 << 1,
        FlipY = 1 << 2,
        FlipXY = FlipX | FlipY,
        NoShadows = 1 << 3,
        NoSelfShadows = 1 << 4,
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
        void push(const Math::Vec2<float>& pos, const Math::Rect& tex_coords, size_t tex, float rotation = 0, unsigned int flags = 0);
        void push(const Math::Vec2<int>& pos, const Math::Color& color, unsigned int radius, float intensity, float decay_rate);
        void render(Canvas& canvas, TextureGroup& textures);
        void clear(Canvas& canvas);

    private:
        OpenGL::Program program, shadow_program, light_program;
        OpenGL::Buffer canvas_uniform, light_uniform;
        OpenGL::VertexArray vao, light_vao;

        struct Quad {
        Math::Vec2<float> pos;
        Math::Rect tex_coords;
            unsigned int tex, flags;
            float rot;
        };

        struct Light {
        Math::Vec2<int> pos;
            unsigned int radius;
            float intensity, decay_rate;
            float p1, p2, p3;
        Math::Color color;
        };

        std::vector<Quad> quads;
        std::vector<Light> lights;
    };

    class SimpleRenderer {
    public:
        SimpleRenderer();
        void render(Canvas& canvas, const Math::Vec2<int>& pos, const Math::Rect& tex_coords, OpenGL::Texture& tex,
                    unsigned int flags = 0);
        void clear(Canvas& canvas);

    private:
        OpenGL::Program program;
        OpenGL::Uniform canvas_size, coords_uniform, pos_uniform, flags_uniform;
        OpenGL::VertexArray vao;
    };

}
