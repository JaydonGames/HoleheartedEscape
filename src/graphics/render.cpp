#include <array>
#include <cassert>
#include <cmath>
#include <memory>
#include <string>
#include "graphics/render.hpp"
#include "assets.hpp"

// TODO REMOVE
#include <GL/glew.h>

namespace Render {
    inline constexpr unsigned int camera_uniform_binding = 10;

    Camera::Camera() {
        ubo.create(OpenGL::Buffer::uniform);
    }

    void Camera::set(int x, int y, float zoom) {
        struct {
            int x, y;
            float zoom;
        } data{x, y, zoom};
        ubo.store(&data, sizeof(data));
    }

    void Camera::bind() {
        ubo.bind(camera_uniform_binding);
    }

    BatchRenderer::BatchRenderer() {
        OpenGL::Shader vert{OpenGL::Shader::vert, Shaders::batch_vert};
        OpenGL::Shader frag{OpenGL::Shader::frag, Shaders::batch_frag};
        OpenGL::Shader tcs{OpenGL::Shader::tcs, Shaders::batch_tcs};
        OpenGL::Shader tes{OpenGL::Shader::tes, Shaders::batch_tes};
        this->program.create();
        this->program.attach(vert);
        this->program.attach(frag);
        this->program.attach(tcs);
        this->program.attach(tes);
        this->program.link();
        this->program.bind();

        this->program.bind_uniform_block("camera_block", camera_uniform_binding);
        this->canvas_size = this->program.get_uniform("canvas");

        this->vao.create();
        this->vao.attach_vbo(0, sizeof(Quad));
        this->vao.vert_attr<int[2]>(0);
        this->vao.vert_attr<int[2]>(1);
        this->vao.vert_attr<int[2]>(2);
        this->vao.vert_attr<unsigned int[1]>(3);
        this->vao.vert_attr<unsigned int[1]>(4);

        for (int i = 0; i < 16; ++i) //TODO REMOVE
            this->program.get_uniform(("tex[" + std::to_string(i) + "]").c_str()).store(i);

        glFrontFace(GL_CCW);
    }

    void BatchRenderer::push(const Vec2& pos, const Rect& coords, Texture& tex, unsigned int flags) {
        long id = -1;
        for (size_t i = 0; i < textures.size(); ++i)
            if (textures[i] == &tex)
                id = i;
        if (id == -1){
            id = this->textures.size();
            this->textures.push_back(&tex);
        }

        this->quads.emplace_back(pos, coords, this->textures.size(), flags);
    }

    void BatchRenderer::render(Canvas& canvas) {
        OpenGL::Context::set_canvas_size(canvas.x, canvas.y);
        this->canvas_size.store(canvas.x, canvas.y);
        this->vao.get_vbo().store(this->quads.data(), this->quads.size()*sizeof(Quad));

        for (size_t i = 0; i < this->textures.size(); ++i) //TODO REMOVE
            this->textures[i]->bind(i);

        this->program.draw_patches(this->vao, this->quads.size(), 1);
        this->quads.clear();
        this->textures.clear();
    }

    void BatchRenderer::clear(Canvas& canvas) {
        canvas.fbo.bind();
        OpenGL::Context::clear();
        canvas.fbo.unbind();
    }

}
