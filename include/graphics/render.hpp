#pragma once
#include <functional>
#include <vector>
#include "opengl.hpp"

namespace Render {
    using OpenGL::Texture;

    struct Rect {
        unsigned int x, y, w, h;
    };


    class BatchRenderer {
    public:
        BatchRenderer();
        void push(const Rect& pos, const Rect& tex_coords, Texture& tex);
        void render();
        void set_canvas(unsigned int x, unsigned int y);

    private:
        void fill_vao(size_t quad_count);

        struct Quad {
            Rect pos, tex_coords;
            unsigned int tex;
            unsigned int p1, p2, p3;
        };

        typedef std::unordered_map<Texture*, size_t> index_map_t;
        std::vector<Quad> render_queues;
        std::vector<size_t> queue_ends;
        std::vector<std::reference_wrapper<Texture>> textures;
        index_map_t texture_indices;

        OpenGL::Program program;
        OpenGL::Uniform screen_size, tex_sizes;
        OpenGL::VertexArray vao;
        OpenGL::Buffer ubo;

        size_t vao_size = 0;
        int max_textures;
    };

}


