#include <cassert>
#include <string>
#include "graphics/render.hpp"
#include <SDL_image.h>
#include <SDL_surface.h>
#include "assets.hpp"

namespace Render {
    inline constexpr unsigned int camera_uniform_binding = 10;

    struct Camera::Internal {
        int x, y;
        float zoom;
    };

    Camera::Camera() {
        ubo.create(OpenGL::Buffer::uniform);
    }

    Camera::Camera(const Camera& other) {
        *this = other;
    }

    Camera& Camera::operator=(const Camera& other){
        Internal data;
        ubo.create(OpenGL::Buffer::uniform);
        other.ubo.get_data(&data, sizeof(data));
        ubo.store(&data, sizeof(data));
        return *this;
    }

    void Camera::set(int x, int y, float zoom) {
        Internal data{x, y, zoom};
        ubo.store(&data, sizeof(data));
    }

    void Camera::use() {
        ubo.bind(camera_uniform_binding);
    }

    TextureGroup::TextureGroup(TextureGroup& other) {
        this->textures = other.textures;
        if (other.tex_arr)
            this->finalize();
    }

    size_t TextureGroup::push_back(uint8_t data[], size_t size) {
        textures.emplace_back(data, size);
        return textures.size() - 1;
    }

    size_t TextureGroup::push_back(Textures::asset_t& texture) {
        return this->push_back(texture.data, texture.length);
    }

    void TextureGroup::bind(unsigned int binding) {
        this->tex_arr.bind(binding);
    }

    void TextureGroup::finalize() {
        int width = 0, height = 0;
        for (size_t i = 0; i < this->textures.size(); ++i) {
            SDL_Surface* surface = IMG_Load_RW(SDL_RWFromConstMem(textures[i].data, textures[i].size), true);
            width = std::max(width, surface->w);
            height = std::max(width, surface->h);
            SDL_FreeSurface(surface);
        }

        tex_arr.create(OpenGL::Texture::texarr2d);
        tex_arr.alloc(width, height, this->textures.size());

        for (size_t i = 0; i < this->textures.size(); ++i)
            tex_arr.load(textures[i].data, textures[i].size, i, true);

        tex_arr.set_wrap_x(OpenGL::Texture::repeat);
        tex_arr.set_wrap_y(OpenGL::Texture::repeat);
        tex_arr.set_mag_filter(OpenGL::Texture::nearest);
        tex_arr.set_min_filter(OpenGL::Texture::nearest);
    }

    BatchRenderer::BatchRenderer() {
        OpenGL::Shader quad_vert{OpenGL::Shader::vert, Shaders::batch_vert};
        OpenGL::Shader quad_tcs{OpenGL::Shader::tcs, Shaders::batch_tcs};
        OpenGL::Shader quad_tes{OpenGL::Shader::tes, Shaders::batch_tes};
        OpenGL::Shader quad_frag{OpenGL::Shader::frag, Shaders::batch_frag};

        OpenGL::Shader shadow_tcs{OpenGL::Shader::tcs, Shaders::shadow_tcs};
        OpenGL::Shader shadow_tes{OpenGL::Shader::tes, Shaders::shadow_tes};
        OpenGL::Shader shadow_frag{OpenGL::Shader::frag, Shaders::shadow_frag};

        OpenGL::Shader light_vert{OpenGL::Shader::vert, Shaders::light_vert};
        OpenGL::Shader light_frag{OpenGL::Shader::frag, Shaders::light_frag};

        this->program.create();
        this->program.attach(quad_vert);
        this->program.attach(quad_tcs);
        this->program.attach(quad_tes);
        this->program.attach(quad_frag);
        this->program.link();

        this->shadow_program.create();
        this->shadow_program.attach(quad_vert);
        this->shadow_program.attach(shadow_tcs);
        this->shadow_program.attach(shadow_tes);
        this->shadow_program.attach(shadow_frag);
        this->shadow_program.link();

        this->light_program.create();
        this->light_program.attach(light_vert);
        this->light_program.attach(light_frag);
        this->light_program.link();

        this->program.bind_uniform_block("camera_block", camera_uniform_binding);
        this->program.bind_uniform_block("canvas_block", 0);
        this->shadow_program.bind_uniform_block("camera_block", camera_uniform_binding);
        this->shadow_program.bind_uniform_block("canvas_block", 0);
        this->shadow_program.bind_uniform_block("light_block", 1);
        this->light_program.bind_uniform_block("camera_block", camera_uniform_binding);
        this->light_program.bind_uniform_block("canvas_block", 0);
        this->light_program.bind_uniform_block("light_block", 1);

        this->light_uniform.create(OpenGL::Buffer::uniform);
        this->canvas_uniform.create(OpenGL::Buffer::uniform);

        this->vao.create();
        this->vao.attach_vbo(0, sizeof(Quad));
        this->vao.vert_attr<int[2]>(0, offsetof(Quad, pos));
        this->vao.vert_attr<int[2]>(1, offsetof(Quad, tex_coords));
        this->vao.vert_attr<int[2]>(2, offsetof(Quad, tex_coords) + offsetof(Rect, w));
        this->vao.vert_attr<unsigned int[1]>(3, offsetof(Quad, tex));
        this->vao.vert_attr<unsigned int[1]>(4, offsetof(Quad, flags));

        this->light_vao.create();
        this->light_vao.attach_ebo();
        this->light_vao.attach_vbo(0, sizeof(float[2]));
        this->light_vao.vert_attr<float[2]>(0, 0);

        float vertices[] = {
            0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f,
        };

        unsigned int indices[] = {
            0, 1, 2, 0, 3, 2,
        };

        this->light_vao.get_vbo().store(vertices, sizeof(vertices), false);
        this->light_vao.get_ebo().store(indices, sizeof(indices), false);

        OpenGL::Context::blending(true);
    }

    void BatchRenderer::push(const Vec2& pos, const Rect& coords, size_t tex, unsigned int flags) {
        this->quads.emplace_back(pos, coords, tex, flags);
    }

    void BatchRenderer::push(const Vec2& pos, const Color& color, unsigned int radius, float intensity) {
        this->lights.emplace_back(pos, radius, intensity, color);
    }

    void BatchRenderer::render(Canvas& canvas, TextureGroup& textures) {
        this->program.use();
        OpenGL::Context::set_canvas_size(canvas.x, canvas.y);
        this->canvas_uniform.store(&canvas.x, sizeof(unsigned int[2]));
        this->canvas_uniform.bind(0);
        this->light_uniform.bind(1);
        this->vao.get_vbo().store(this->quads.data(), this->quads.size() * sizeof(Quad));

        if (!canvas.internal_fbo) {
            canvas.internal_color.create(OpenGL::Texture::tex2d);
            canvas.internal_color.alloc(canvas.x, canvas.y, OpenGL::format<8>(OpenGL::Format::R));
            canvas.internal_fbo.create();
            canvas.internal_fbo.attach(OpenGL::Framebuffer::COLOR, canvas.internal_color);
        } else {
            canvas.internal_fbo.bind();
            OpenGL::Context::clear();
        }

        textures.bind(0);
        canvas.fbo.bind();
        this->program.draw_patches(this->vao, this->quads.size(), 1);

        canvas.internal_color.bind(0);
        for (Light& light : this->lights) {
            this->light_uniform.store(&light, sizeof(light));

            canvas.internal_fbo.bind();
            this->shadow_program.draw_patches(this->vao, this->quads.size(), 1);

            canvas.fbo.bind();
            this->light_program.draw_tri(this->light_vao, 6);
        }

        canvas.fbo.unbind();
        this->quads.clear();
        this->lights.clear();
    }

    void BatchRenderer::clear(Canvas& canvas) {
        canvas.fbo.bind();
        OpenGL::Context::clear();
        canvas.fbo.unbind();
    }

    SimpleRenderer::SimpleRenderer() {
        OpenGL::Shader vert{OpenGL::Shader::vert, Shaders::simple_vert};
        OpenGL::Shader frag{OpenGL::Shader::frag, Shaders::simple_frag};
        this->program.create();
        this->program.attach(vert);
        this->program.attach(frag);
        this->program.link();

        this->program.bind_uniform_block("camera_block", camera_uniform_binding);
        this->canvas_size = this->program.get_uniform("canvas");
        this->coords_uniform = this->program.get_uniform("coords");
        this->pos_uniform = this->program.get_uniform("position");
        this->flags_uniform = this->program.get_uniform("flags");

        this->vao.create();
        this->vao.attach_ebo();
        this->vao.attach_vbo(0, sizeof(float[2]));
        this->vao.vert_attr<float[2]>(0, 0);

        float vertices[] = {
            0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f,
        };

        unsigned int indices[] = {
            0, 1, 2, 0, 3, 2,
        };

        this->vao.get_vbo().store(vertices, sizeof(vertices), false);
        this->vao.get_ebo().store(indices, sizeof(indices), false);
    }

    void SimpleRenderer::render(Canvas& canvas, const Vec2& pos, const Rect& tex_coords, OpenGL::Texture& tex,
                                unsigned int flags) {
        this->program.use();
        OpenGL::Context::set_canvas_size(canvas.x, canvas.y);
        this->canvas_size.store(canvas.x, canvas.y);
        this->coords_uniform.store(tex_coords.x, tex_coords.y, tex_coords.w, tex_coords.h);
        this->pos_uniform.store(pos.x, pos.y);
        this->flags_uniform.store(flags);

        tex.bind(0);
        canvas.fbo.bind();
        this->program.draw_tri(this->vao, 6);
        canvas.fbo.unbind();
    }

    void SimpleRenderer::clear(Canvas& canvas) {
        canvas.fbo.bind();
        OpenGL::Context::clear();
        canvas.fbo.unbind();
    }

}
