#pragma once
#include <functional>
#include <vector>
#include "opengl.hpp"

namespace Render {
    using OpenGL::Texture;

    struct Rect {
        int x, y, w, h;
    };



    class Camera {
    public:
        Camera(OpenGL::Program& program, OpenGL::Uniform camera)
            : program(program), camera(camera) {}
        template<typename T>
        Camera(T& renderer) : Camera(renderer.get_camera()){}
        void move(int x, int y);

    private:
        OpenGL::Program& program;
        OpenGL::Uniform camera;
    };


    enum Flags {
        FlipDiagonally = 1<<0,
        FlipX = 1<<1,
        FlipY = 1<<2
    };


    class BatchRenderer {
    public:
        BatchRenderer();
        void push(const Rect& pos, const Rect& tex_coords, Texture& tex, unsigned int flags);
        void render();
        void set_canvas(unsigned int x, unsigned int y);
        inline Camera get_camera(){
            return Camera{this->program, this->program.get_uniform("camera")};
        }

    private:
        void fill_vao(size_t quad_count);

        struct Quad {
            Rect pos, tex_coords;
            unsigned int tex, flags;
            unsigned int p1, p2;
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


