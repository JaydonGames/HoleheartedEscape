#include <cassert>
#include <string>
#include "graphics/render.hpp"
#include <SDL_image.h>
#include <SDL_surface.h>
#include "assets.hpp"

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

    TextureGroup::TextureGroup(TextureGroup& other) {
        this->textures = other.textures;
        if (other.tex_arr)
            this->finalize();
    }

    size_t TextureGroup::push_back(uint8_t data[], size_t size) {
        textures.emplace_back(data, size);
        return textures.size() - 1;
    }

    size_t TextureGroup::push_back(Textures::asset_t& texture){
        return this->push_back(texture.data, texture.length);
    }

    void TextureGroup::bind(unsigned int binding){
        this->tex_arr.bind(binding);
    }

    void TextureGroup::finalize(){
        int width = 0, height = 0;
        for (size_t i = 0; i < this->textures.size(); ++i){
            SDL_Surface *surface = IMG_Load_RW(SDL_RWFromConstMem(textures[i].data, textures[i].size), true);
            width = std::max(width, surface->w);
            height = std::max(width, surface->h);
            SDL_FreeSurface(surface);
        }

        tex_arr.create(OpenGL::Texture::texarr2d);
        tex_arr.alloc(width, height, this->textures.size());

        for (size_t i = 0; i < this->textures.size(); ++i)
            tex_arr.load(textures[i].data, textures[i].size, i);

        tex_arr.set_wrap_x(OpenGL::Texture::repeat);
        tex_arr.set_wrap_y(OpenGL::Texture::repeat);
        tex_arr.set_mag_filter(OpenGL::Texture::nearest);
        tex_arr.set_min_filter(OpenGL::Texture::nearest);
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
        this->vao.vert_attr<int[2]>(0, offsetof(Quad, pos));
        this->vao.vert_attr<int[2]>(1, offsetof(Quad, tex_coords));
        this->vao.vert_attr<int[2]>(2, offsetof(Quad, tex_coords) + offsetof(Rect, w));
        this->vao.vert_attr<unsigned int[1]>(3, offsetof(Quad, tex));
        this->vao.vert_attr<unsigned int[1]>(4, offsetof(Quad, flags));
    }

    void BatchRenderer::push(const Vec2& pos, const Rect& coords, size_t tex, unsigned int flags) {
        this->quads.emplace_back(pos, coords, tex, flags);
    }

    void BatchRenderer::render(Canvas& canvas, TextureGroup& textures) {
        OpenGL::Context::set_canvas_size(canvas.x, canvas.y);
        this->canvas_size.store(canvas.x, canvas.y);
        this->vao.get_vbo().store(this->quads.data(), this->quads.size() * sizeof(Quad));

        textures.bind(0);
        canvas.fbo.bind();
        this->program.draw_patches(this->vao, this->quads.size(), 1);
        canvas.fbo.unbind();
        this->quads.clear();
    }

    void BatchRenderer::clear(Canvas& canvas) {
        canvas.fbo.bind();
        OpenGL::Context::clear();
        canvas.fbo.unbind();
    }

}
