#pragma once
#include <unordered_map>
#include <vector>
#include "opengl.hpp"
#include "types.hpp"

namespace Render {
    using OpenGL::Texture;


    class Camera {
    public:
        Camera(OpenGL::Program& program, OpenGL::Uniform camera)
            : program(program), camera(camera) {}
        template<typename T>
        Camera(T& renderer) : Camera(renderer.get_camera()){}
        void set(int x, int y);
        void set(int x, int y, float zoom);

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
        void push(const Vec2& pos, const Rect& tex_coords, Texture& tex, unsigned int flags = 0);
        void render();
        void render(OpenGL::Framebuffer& fbo);
        void clear();
        void clear(OpenGL::Framebuffer& fbo);
        void set_canvas(unsigned int x, unsigned int y);
        inline Camera get_camera(){
            return Camera{this->program, this->program.get_uniform("camera")};
        }


    private:
        Vec2 canvas;
        void fill_vao(size_t quad_count);

        OpenGL::Program program;
        OpenGL::Uniform screen_size, tex_sizes;
        OpenGL::VertexArray vao;
        OpenGL::Buffer ubo;
        int max_textures;
        static constexpr unsigned int max_quads = 512;
        
        struct Quad {
            Vec2 pos;
            Rect tex_coords;
            unsigned int tex, flags;
        };

        struct QueueIndices {
            size_t quad_end, tex_end;
        };

        std::vector<QueueIndices> queues;
        std::vector<Quad> quads;
        std::vector<Texture*> textures;
        std::unordered_map<Texture*, size_t> texture_cache;

        struct Queue {
            BatchRenderer& renderer;
            size_t queue;
            inline Queue& operator++(){
                ++this->queue;
                return *this;
            }

            inline Queue& operator*(){
                return *this;
            }

            inline bool operator !=(const Queue& other){
                return this->queue != other.queue;
            }

            inline void render();
            inline void bind_textures();
        };

        inline Queue begin(){
            return {*this, 0};
        }

        inline Queue end(){
            return {*this, this->queues.size()};
        }

        inline void push(Quad);
        inline void new_queue();
        inline void clean();
    };



}


