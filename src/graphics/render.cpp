#include <array>
#include <cassert>
#include <cmath>
#include <memory>
#include <string>
#include "graphics/render.hpp"
#include "assets.hpp"

namespace Render {

    void Camera::set(int x, int y) {
        this->program.bind();
        this->camera.store(float(x), float(y));
    }

    void Camera::set(int x, int y, float zoom) {
        this->program.bind();
        this->camera.store(float(x), float(y), zoom);
    }

    void BatchRenderer::Queue::bind_textures() {
        size_t begin = this->queue ? this->renderer.queues[queue - 1].tex_end : 0;
        size_t end = this->renderer.queues[queue].tex_end;
        std::vector<std::pair<unsigned int, unsigned int>> sizes;
        for (int i = 0; begin + i < end; ++i) {
            OpenGL::Texture& tex = *this->renderer.textures[begin + i];
            tex.bind(i);
            sizes.emplace_back(tex.width(), tex.height());
        }
        this->renderer.tex_sizes.store<2>(&sizes[0].first, sizes.size());
    }

    void BatchRenderer::Queue::render() {
        size_t quad_begin = this->queue ? this->renderer.queues[queue - 1].quad_end : 0;
        unsigned int size = this->renderer.queues[queue].quad_end - quad_begin;
        assert(size <= BatchRenderer::max_quads);
        assert(size * sizeof(Quad) <= 16 * (2 << 9));
        this->renderer.ubo.store(OpenGL::Buffer::Type::uniform, this->renderer.quads.data() + quad_begin,
                                 size * sizeof(Quad));
        this->renderer.vao.draw_triangles(size * 6);
    }

    OpenGL::Shader get_shader(bool frag) {
        std::string shader{frag ? Shaders::batch_render_frag : Shaders::batch_render_vert};
        shader = shader.replace(shader.find("@"), 1, std::to_string(OpenGL::max_textures()));
        return OpenGL::Shader{frag ? OpenGL::Shader::frag : OpenGL::Shader::vert, shader.c_str()};
    }

    BatchRenderer::BatchRenderer()
        : max_textures(OpenGL::max_textures()),
          program{get_shader(0), get_shader(1)},
          screen_size(program.get_uniform("screen")),
          tex_sizes(program.get_uniform("tex_wh")) {
        program.bind();
        for (int i = 0; i < this->max_textures; ++i)
            this->program.get_uniform(("tex[" + std::to_string(i) + "]").c_str()).store(i);
        program.bind_uniform_buffer("quad_block", 0);
        this->vao.bind();
        this->fill_vao(BatchRenderer::max_quads);
    }

    void BatchRenderer::push(const Vec2& pos, const Rect& coords, Texture& tex, unsigned int flags) {
        unsigned int tex_id;
        if (!this->texture_cache.contains(&tex)) {
            tex_id = this->textures.size() - (this->queues.empty() ? 0 : this->queues.back().tex_end);
            this->textures.push_back(&tex);
            this->texture_cache[&tex] = tex_id;
        } else
            tex_id = this->texture_cache[&tex];
        this->push({pos, coords, tex_id, flags});
    }

    void BatchRenderer::push(Quad quad) {
        this->quads.emplace_back(quad);
        QueueIndices prev = this->queues.empty() ? QueueIndices{0, 0} : this->queues.back();
        if (this->quads.size() - prev.quad_end >= BatchRenderer::max_quads ||
            this->textures.size() - prev.tex_end >= this->max_textures)
            this->new_queue();
    }

    void BatchRenderer::new_queue() {
        if (this->quads.size() == (this->queues.empty() ? 0 : this->queues.back().quad_end))
            return;
        this->queues.emplace_back(this->quads.size(), this->textures.size());
        this->texture_cache.clear();
    }

    void BatchRenderer::clean() {
        this->queues.clear();
        this->quads.clear();
        this->textures.clear();
        this->texture_cache.clear();
    }

    void BatchRenderer::render() {
        this->program.bind();
        this->vao.bind();
        this->ubo.bind(OpenGL::Buffer::Type::uniform);
        this->ubo.bind(OpenGL::Buffer::Type::uniform, 0);
        this->new_queue();

        OpenGL::Context::set_canvas_size(canvas.x, canvas.y);

        for (Queue queue : *this) {
            queue.bind_textures();
            queue.render();
        }

        this->clean();
    }

    void BatchRenderer::render(OpenGL::Framebuffer& fbo) {
        fbo.bind();
        this->render();
        fbo.unbind();
    }

    void BatchRenderer::clear() {
        OpenGL::Context::clear();
    }


    void BatchRenderer::clear(OpenGL::Framebuffer& fbo) {
        fbo.bind();
        this->clear();
        fbo.unbind();
    }

    void BatchRenderer::fill_vao(size_t quad_count) {
        struct Vertex {
            unsigned int x, y;
        };

        typedef std::array<Vertex, 4> QuadVerts;
        typedef std::array<unsigned int, 6> QuadIndices;

        auto quads = std::make_unique<QuadVerts[]>(quad_count);
        auto indices = std::make_unique<QuadIndices[]>(quad_count);

        for (unsigned int i = 0; i < quad_count; ++i) {
            quads[i] = {
                Vertex{1, 1},
                Vertex{1, 0},
                Vertex{0, 0},
                Vertex{0, 1},
            };

            unsigned int vertex = i * 4;
            indices[i] = {vertex + 0, vertex + 1, vertex + 3, vertex + 1, vertex + 2, vertex + 3};
        }

        OpenGL::Buffer::store(OpenGL::Buffer::vertex, quads.get(), sizeof(QuadVerts) * quad_count);
        OpenGL::Buffer::store(OpenGL::Buffer::index, &indices.get()[0][0], sizeof(QuadIndices) * quad_count);
        this->vao.vert_attr<unsigned int[2]>(0);
    }

    void BatchRenderer::set_canvas(unsigned int x, unsigned int y) {
        this->program.bind();
        this->screen_size.store(x, y);
        canvas.x = x;
        canvas.y = y;
    }
}
