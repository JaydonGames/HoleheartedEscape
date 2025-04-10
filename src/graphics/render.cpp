#include <array>
#include <cmath>
#include <memory>
#include <string>
#include "graphics/render.hpp"
#include "assets.hpp"

namespace Render {

    /* Replaces '@' in the shader with the max texture count. */
    std::string get_shader(bool frag){
        std::string shader{frag ? Shaders::batch_render_frag : Shaders::batch_render_vert};
        size_t index = shader.find("@");
        return std::string(shader.c_str(), index) +
            std::to_string(OpenGL::max_textures()) +
            std::string(shader.c_str()+index+1, shader.size()-index-1);
    }

    BatchRenderer::BatchRenderer() : program(get_shader(0).c_str(), get_shader(1).c_str()), screen_size(program.get_uniform("screen")), tex_sizes(program.get_uniform("tex_wh")) {
        this->max_textures = OpenGL::max_textures();
        program.bind();
        for (int i = 0; i < this->max_textures; ++i)
            this->program.get_uniform(("tex[" + std::to_string(i) + "]").c_str()).store(i);
        program.bind_uniform_buffer("quad_block", 0);
    } 

    void BatchRenderer::push(const Rect& pos, const Rect& tex_coords, Texture& tex){
        index_map_t::iterator iter = this->texture_indices.find(&tex);

        if (iter != this->texture_indices.end()){
            render_queues.emplace_back(pos, tex_coords, iter->second);
        }
        else{
            int index = this->textures.size() % this->max_textures;
            this->textures.emplace_back(tex);
            render_queues.emplace_back(pos, tex_coords, index);
            this->texture_indices[&tex] = index;

            if (index == this->max_textures - 1) {
                queue_ends.push_back(this->render_queues.size());
                this->texture_indices.clear();
            }
        }
    }

    void BatchRenderer::render(){
        this->program.bind();
        this->vao.bind();
        this->ubo.bind(OpenGL::Buffer::Type::uniform);
        this->ubo.bind(OpenGL::Buffer::Type::uniform, 0);

        if (vao_size < render_queues.size())
            this->fill_vao(2*render_queues.size());

        std::vector<std::pair<unsigned int, unsigned int>> tex_sizes;

        //TODO: implement max 500 quads for each render call
        size_t group_count = std::ceil(this->textures.size()/static_cast<double>(max_textures));
        for (size_t group = 0; group < group_count; ++group){
            size_t texture_begin = group*max_textures;
            size_t texture_end = std::min(texture_begin+max_textures, this->textures.size());

            for (size_t texture = texture_begin; texture < texture_end; ++texture){
                OpenGL::Texture& tex = this->textures[texture].get();
                tex.bind(texture-texture_begin);
                tex_sizes.push_back({tex.width(), tex.height()});
            }

            size_t queue_begin = group ? queue_ends[group-1] : 0;
            size_t size = (queue_ends.size() > group ? queue_ends[group] : render_queues.size()) - queue_begin;
            this->ubo.fill(OpenGL::Buffer::Type::uniform, render_queues.data() + queue_begin, size*sizeof(Quad));
            this->tex_sizes.store<2>(&tex_sizes[0].first, tex_sizes.size());
            this->vao.draw_triangles(size*6);

            tex_sizes.clear();
        }

        textures.clear();
        queue_ends.clear();
        render_queues.clear();
        texture_indices.clear();
    }


    void BatchRenderer::fill_vao(size_t quad_count){
        struct Vertex {
            unsigned int x, y;
        };

        typedef std::array<Vertex, 4> QuadVerts;
        typedef std::array<unsigned int, 6> QuadIndices;

        auto quads = std::make_unique<QuadVerts[]>(quad_count);
        auto indices = std::make_unique<QuadIndices[]>(quad_count);

        for (unsigned int i = 0; i < quad_count; ++i){
            quads[i] = {
                Vertex{1, 1},
                Vertex{1, 0},
                Vertex{0, 0},
                Vertex{0, 1},
            };

            unsigned int vertex = i*4;
            indices[i] = {
                vertex+0, vertex+1, vertex+3,
                vertex+1, vertex+2, vertex+3
            };
        }

        OpenGL::Buffer::fill(OpenGL::Buffer::vertex, quads.get(), sizeof(QuadVerts) * quad_count);
        OpenGL::Buffer::fill(OpenGL::Buffer::index, &indices.get()[0][0], sizeof(QuadIndices) * quad_count);
        this->vao.vert_attr<unsigned int[2]>(0);
        this->vao_size = quad_count;
    }

    void BatchRenderer::set_canvas(unsigned int x, unsigned int y){
        this->program.bind();
        this->screen_size.store(x, y);
        OpenGL::Context::set_canvas_size(x, y);
    }
}
