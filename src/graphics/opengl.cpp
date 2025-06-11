#include "graphics/opengl.hpp"
#include "assets.hpp"
#include <GL/glew.h>
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_surface.h>
#include <SDL_video.h>
#include <cassert>
#include <iostream>

namespace OpenGL {

    int max_textures() {
        int max;
        glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &max);
        return max;
    }

    InitError::InitError(const std::string &what)
        : std::runtime_error(what) {}

    InitError::InitError(const char *what)
        : std::runtime_error(what) {}

    InitError::InitError(const unsigned char *what)
        : std::runtime_error(std::string(what, what + strlen(reinterpret_cast<const char *>(what)))) {}

    void opengl_debug_out(GLenum source, GLenum type, unsigned int id, GLenum severity, GLsizei length,
                          const char *message, const void *) {
        if (id == 131169 || id == 131185 || id == 131218 || id == 131204)
            return;
        std::cout << "OpenGL message (" << id << "): " << message << std::endl;
    }

    bool Context::debug = false;

    Context::Context(SDL_Window *window, int major, int minor, bool core)
        : window(window) {
        if (Context::debug)
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, major);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, minor);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,
                            core ? SDL_GL_CONTEXT_PROFILE_CORE : SDL_GL_CONTEXT_PROFILE_COMPATIBILITY);
        this->context = SDL_GL_CreateContext(window);
        if (!this->context)
            throw InitError{SDL_GetError()};
        ::glewExperimental = true;
        unsigned int err = glewInit();
        if (err)
            throw InitError{glewGetErrorString(err)};

        if (Context::debug) {
            glEnable(GL_DEBUG_OUTPUT);
            glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
            glDebugMessageCallback(opengl_debug_out, nullptr);
            glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
        }
    }

    void Context::wireframe(bool enable) {
        if (enable) {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            glLineWidth(5.0f);
        } else
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }

    void Context::blending(bool enable) {
        if (enable) {
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        } else
            glDisable(GL_BLEND);
    }

    bool Context::enable_vsync() {
        return SDL_GL_SetSwapInterval(1) >= 0;
    }

    void Context::set_canvas_size(int x, int y) {
        glViewport(0, 0, x, y);
    }

    void Context::clear() {
        glClear(GL_COLOR_BUFFER_BIT);
    }

    void Context::set_clear_color(float r, float g, float b, float a) {
        glClearColor(r, g, b, a);
    }

    void Context::sync() {
        glFinish();
    }

    void Context::swap_buffer() {
        SDL_GL_SwapWindow(this->window);
    }

    unsigned int Format::to_opengl() const {
        constexpr unsigned int typed_fmt[3][4][6] = {
            {/* unsigned integral */
             {GL_R8UI, GL_RG8UI, GL_RGB8UI, GL_RGBA8UI, 0, 0},
             {GL_R16UI, GL_RG16UI, GL_RGB16UI, GL_RGBA16UI, 0, 0},
             {0, 0, 0, 0, 0, 0},
             {GL_R32UI, GL_RG32UI, GL_RGB32UI, GL_RGBA32UI, 0, 0}},
            {/* signed integral */
             {GL_R8I, GL_RG8I, GL_RGB8I, GL_RGBA8I, 0, 0},
             {GL_R16I, GL_RG16I, GL_RGB16I, GL_RGBA16I, 0, 0},
             {0, 0, 0, 0, 0, 0},
             {GL_R32I, GL_RG32I, GL_RGB32I, GL_RGBA32I, 0, 0}},
            {/* floating point */
             {0, 0, 0, 0, 0, 0},
             {GL_R16F, GL_RG16F, GL_RGB16F, GL_RGBA16F, 0, 0},
             {0, 0, 0, 0, 0, 0},
             {GL_R32F, GL_RG32F, GL_RGB32F, GL_RGBA32F, GL_DEPTH_COMPONENT32F, GL_DEPTH32F_STENCIL8}}};

        constexpr unsigned int norm_fmt[8][6] = {
            {0, 0, 0, GL_RGBA2, 0, 0}, {0, 0, GL_RGB4, GL_RGBA4, 0, 0},
            {0, 0, GL_RGB5, 0, 0, 0},  {GL_R8, GL_RG8, GL_RGB8, GL_RGBA8, 0, 0},
            {0, 0, GL_RGB10, 0, 0, 0}, {0, 0, GL_RGB12, GL_RGBA12, 0, 0},
            {0, 0, 0, 0, 0, 0},        {GL_R16, GL_RG16, GL_RGB16, GL_RGBA16, GL_DEPTH_COMPONENT16, 0},
        };

        constexpr unsigned int norm_signed_fmt[2][6] = {
            {GL_R8_SNORM, GL_RG8_SNORM, GL_RGB8_SNORM, GL_RGBA8_SNORM, 0, 0},
            {GL_R16_SNORM, GL_RG16_SNORM, GL_RGB16_SNORM, GL_RGBA16_SNORM, 0, 0},
        };

        unsigned int format;
        if (type == Typed)
            format = typed_fmt[typed.is_signed + typed.is_floating_point][typed.size - 1][typed.component];
        if (type == Normalized) {
            if (norm.is_signed)
                format = norm_signed_fmt[norm.bits == 16][norm.component];
            else if (norm.bits == 5)
                format = norm_fmt[2][norm.component];
            else if ((norm.bits == 24 || norm.bits == 32) && norm.component == Components::DEPTH)
                format = norm.bits == 24 ? GL_DEPTH_COMPONENT24 : GL_DEPTH_COMPONENT32;
            else if (norm.bits == 24 && norm.component == Components::DEPTH_STENCIL)
                format = GL_DEPTH24_STENCIL8;
            else
                format = norm_fmt[norm.bits / 2 - 1][norm.component];
        }

        assert(format);
        return format;
    }

    Buffer::Buffer(Type type) {
        this->create(type);
    }

    Buffer::Buffer(Type type, const void *arr, size_t size, bool dynamic) {
        this->create(type);
        this->store(arr, size, dynamic);
    }

    Buffer::Buffer(Type type, size_t size) {
        this->create(type);
        this->alloc(size);
    }

    void Buffer::create(Type type) {
        this->destroy();
        this->type = type;
        glCreateBuffers(1, &this->id);
    }

    void Buffer::destroy() {
        if (this->id)
            glDeleteBuffers(1, &this->id);
        this->id = 0;
    }

    void Buffer::bind(unsigned int index) {
        glBindBufferBase(this->type, index, this->id);
    }

    void Buffer::store(const void *arr, size_t size, bool dynamic) {
        glNamedBufferData(this->id, size, arr, dynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW);
    }

    void Buffer::alloc(size_t size) {
        this->store(nullptr, size, true);
    }

    void Buffer::sync() {
        unsigned int bit;
        switch (type) {
            case shader:
                bit = GL_SHADER_STORAGE_BARRIER_BIT;
            default:
                assert(type == shader);
                return;
        }

        glMemoryBarrier(bit);
    }

    void Buffer::get_data(void *data, size_t size, ptrdiff_t offset) const {
        glGetNamedBufferSubData(this->id, offset, size, data);
    }

    void *Buffer::map(bool writeable) {
        return glMapNamedBuffer(this->id, writeable ? GL_READ_WRITE : GL_READ_ONLY);
    }

    void Buffer::unmap() {
        glUnmapNamedBuffer(this->id);
    }

    VertexArray::VertexArray(ptrdiff_t offset, int stride) {
        this->create();
        this->attach_vbo(offset, stride);
        this->attach_ebo();
    }

    void VertexArray::create() {
        this->destroy();
        glCreateVertexArrays(1, &this->id);
    }

    void VertexArray::destroy() {
        if (this->id)
            glDeleteVertexArrays(1, &this->id);
        this->id = 0;
    }

    void VertexArray::bind() {
        glBindVertexArray(this->id);
    }

    Buffer &VertexArray::attach_vbo(ptrdiff_t offset, int stride, Buffer &&buffer) {
        glVertexArrayVertexBuffer(this->id, 0, buffer.get(), offset, stride);
        this->vbo = std::move(buffer);
        return this->vbo;
    }

    Buffer &VertexArray::attach_ebo(Buffer &&buffer) {
        glVertexArrayElementBuffer(this->id, buffer.get());
        this->ebo = std::move(buffer);
        return this->ebo;
    }

    Buffer &VertexArray::get_vbo() {
        return this->vbo;
    }

    Buffer &VertexArray::get_ebo() {
        return this->ebo;
    }

    template<typename T>
    void VertexArray::vert_attr(unsigned int index, unsigned int offset, bool normalized) {
        constexpr int dimensions = std::extent_v<T>;
        using attr_t = std::remove_cvref_t<decltype(std::declval<T>()[0])>;

        if constexpr (std::is_same_v<attr_t, float>)
            glVertexArrayAttribFormat(this->id, index, dimensions, GL_FLOAT, normalized, offset);
        if constexpr (std::is_same_v<attr_t, unsigned int>)
            glVertexArrayAttribIFormat(this->id, index, dimensions, GL_UNSIGNED_INT, offset);
        if constexpr (std::is_same_v<attr_t, int>)
            glVertexArrayAttribIFormat(this->id, index, dimensions, GL_INT, offset);

        glEnableVertexArrayAttrib(this->id, index);
        glVertexArrayAttribBinding(this->id, index, 0);
    }

    template void VertexArray::vert_attr<float[1]>(unsigned int, unsigned int, bool);
    template void VertexArray::vert_attr<float[2]>(unsigned int, unsigned int, bool);
    template void VertexArray::vert_attr<float[3]>(unsigned int, unsigned int, bool);
    template void VertexArray::vert_attr<float[4]>(unsigned int, unsigned int, bool);
    template void VertexArray::vert_attr<int[1]>(unsigned int, unsigned int, bool);
    template void VertexArray::vert_attr<int[2]>(unsigned int, unsigned int, bool);
    template void VertexArray::vert_attr<int[3]>(unsigned int, unsigned int, bool);
    template void VertexArray::vert_attr<int[4]>(unsigned int, unsigned int, bool);
    template void VertexArray::vert_attr<unsigned int[1]>(unsigned int, unsigned int, bool);
    template void VertexArray::vert_attr<unsigned int[2]>(unsigned int, unsigned int, bool);
    template void VertexArray::vert_attr<unsigned int[3]>(unsigned int, unsigned int, bool);
    template void VertexArray::vert_attr<unsigned int[4]>(unsigned int, unsigned int, bool);

    void Uniform::store(float f1) {
        glUniform1f(this->id, f1);
    }

    void Uniform::store(float f1, float f2) {
        glUniform2f(this->id, f1, f2);
    }

    void Uniform::store(float f1, float f2, float f3) {
        glUniform3f(this->id, f1, f2, f3);
    }

    void Uniform::store(float f1, float f2, float f3, float f4) {
        glUniform4f(this->id, f1, f2, f3, f4);
    }

    void Uniform::store(unsigned int u1) {
        glUniform1ui(this->id, u1);
    }

    void Uniform::store(unsigned int u1, unsigned int u2) {
        glUniform2ui(this->id, u1, u2);
    }

    void Uniform::store(unsigned int u1, unsigned int u2, unsigned int u3) {
        glUniform3ui(this->id, u1, u2, u3);
    }

    void Uniform::store(unsigned int u1, unsigned int u2, unsigned int u3, unsigned int u4) {
        glUniform4ui(this->id, u1, u2, u3, u4);
    }

    void Uniform::store(int i1) {
        glUniform1i(this->id, i1);
    }

    void Uniform::store(int i1, int i2) {
        glUniform2i(this->id, i1, i2);
    }

    void Uniform::store(int i1, int i2, int i3) {
        glUniform3i(this->id, i1, i2, i3);
    }

    void Uniform::store(int i1, int i2, int i3, int i4) {
        glUniform4i(this->id, i1, i2, i3, i4);
    }

    template<size_t vec_size>
    void Uniform::store(const float *arr, size_t count) {
        if constexpr (vec_size == 1)
            glUniform1fv(this->id, count, arr);
        if constexpr (vec_size == 2)
            glUniform2fv(this->id, count, arr);
        if constexpr (vec_size == 3)
            glUniform3fv(this->id, count, arr);
        if constexpr (vec_size == 4)
            glUniform4fv(this->id, count, arr);
    }

    template<size_t vec_size>
    void Uniform::store(const unsigned int *arr, size_t count) {
        if constexpr (vec_size == 1)
            glUniform1uiv(this->id, count, arr);
        if constexpr (vec_size == 2)
            glUniform2uiv(this->id, count, arr);
        if constexpr (vec_size == 3)
            glUniform3uiv(this->id, count, arr);
        if constexpr (vec_size == 4)
            glUniform4uiv(this->id, count, arr);
    }

    template<size_t vec_size>
    void Uniform::store(const int *arr, size_t count) {
        if constexpr (vec_size == 1)
            glUniform1iv(this->id, count, arr);
        if constexpr (vec_size == 2)
            glUniform2iv(this->id, count, arr);
        if constexpr (vec_size == 3)
            glUniform3iv(this->id, count, arr);
        if constexpr (vec_size == 4)
            glUniform4iv(this->id, count, arr);
    }

    template void Uniform::store<1>(const float *arr, size_t count);
    template void Uniform::store<2>(const float *arr, size_t count);
    template void Uniform::store<3>(const float *arr, size_t count);
    template void Uniform::store<4>(const float *arr, size_t count);
    template void Uniform::store<1>(const unsigned int *arr, size_t count);
    template void Uniform::store<2>(const unsigned int *arr, size_t count);
    template void Uniform::store<3>(const unsigned int *arr, size_t count);
    template void Uniform::store<4>(const unsigned int *aloadrr, size_t count);
    template void Uniform::store<1>(const int *arr, size_t count);
    template void Uniform::store<2>(const int *arr, size_t count);
    template void Uniform::store<3>(const int *arr, size_t count);
    template void Uniform::store<4>(const int *arr, size_t count);

    Shader::Shader(Type type, const char *shader) {
        this->create(type, shader);
    }

    void Shader::create(Type type, const char *shader) {
        this->destroy();
        this->id = glCreateShader(type);
        glShaderSource(this->id, 1, &shader, nullptr);
        glCompileShader(this->id);
        int success;
        glGetShaderiv(this->id, GL_COMPILE_STATUS, &success);
        if (!success) {
            char log[512];
            glGetShaderInfoLog(this->id, 512, NULL, log);
            this->id = 0;
            throw InitError{log};
        }
    }

    void Shader::destroy() {
        if (this->id)
            glDeleteShader(this->id);
        this->id = 0;
    }

    Program::Program(std::initializer_list<Shader> shaders) {
        this->create();
        for (const Shader &shader : shaders)
            this->attach(shader);
        this->link();
    }

    Program::Program(std::initializer_list<Shader *> shaders) {
        this->create();
        for (Shader *shader : shaders)
            this->attach(*shader);
        this->link();
    }

    void Program::create() {
        this->destroy();
        this->id = glCreateProgram();
    }

    void Program::destroy() {
        if (this->id)
            glDeleteProgram(this->id);
        this->id = 0;
    }

    void Program::attach(const Shader &shader) {
        glAttachShader(this->id, shader.id);
    }

    void Program::link() {
        glLinkProgram(this->id);
    }

    void Program::use() {
        glUseProgram(this->id);
    }

    Uniform Program::get_uniform(const char *name) {
        return glGetUniformLocation(this->id, name);
    }

    void Program::bind_uniform_block(const char *name, unsigned int binding) {
        glUniformBlockBinding(this->id, glGetUniformBlockIndex(this->id, name), binding);
    }

    void Program::compute(unsigned int x, unsigned int y, unsigned int z) {
        this->use();
        glDispatchCompute(x, y, z);
    }

    void Program::draw_tri(VertexArray &vao, int vert_count) {
        this->use();
        vao.bind();
        glDrawElements(GL_TRIANGLES, vert_count, GL_UNSIGNED_INT, nullptr);
    }

    void Program::draw_patches(VertexArray &vao, int vert_count, int vert_per_patch) {
        glPatchParameteri(GL_PATCH_VERTICES, vert_per_patch);
        this->use();
        vao.bind();
        glDrawArrays(GL_PATCHES, 0, vert_count);
    }

    Texture::Texture(Type type) {
        this->create(type);
    }

    Texture::Texture(Type type, const uint8_t data[], size_t length) {
        this->create(type);
        this->load(data, length);
    }

    Texture::Texture(Type type, const uint8_t data[], size_t length, int level) {
        this->create(type);
        this->load(data, length, level);
    }

    Texture::Texture(Type type, unsigned int width, unsigned int height, Format format) {
        this->create(type);
        this->alloc(width, height, format);
    }

    Texture::Texture(const Textures::asset_t &texture)
        : Texture(tex2d, texture.data, texture.length) {}

    Texture::Texture(Texture &&other)
        : Entity<Texture>(std::move(other)) {
        this->w = other.w;
        this->h = other.h;
    }

    void Texture::create(Type type) {
        this->destroy();
        this->type = type;
        glCreateTextures(type, 1, &this->id);
    }

    void Texture::destroy() {
        if (this->id)
            glDeleteTextures(1, &this->id);
        this->id = 0;
    }

    void Texture::bind(unsigned int index) {
        glActiveTexture(GL_TEXTURE0 + index);
        glBindTexture(this->type, this->id);
    }

    void Texture::load(const uint8_t data[], size_t length, bool flip_y) {
        SDL_Surface *loaded_surface = IMG_Load_RW(SDL_RWFromConstMem(data, length), true);
        SDL_Surface *surface = SDL_ConvertSurfaceFormat(loaded_surface, SDL_PIXELFORMAT_RGBA32, 0);

        this->alloc(surface->w, surface->h);

        if (!flip_y)
            for (size_t y = 0; y < this->h; ++y)
                this->store((uint8_t *)surface->pixels + (this->h - 1 - y) * surface->pitch, surface->w, 1, 0, y);
        else
            this->store((uint8_t *)surface->pixels, surface->w, surface->h);

        SDL_FreeSurface(loaded_surface);
        SDL_FreeSurface(surface);
    }

    void Texture::load(const uint8_t data[], size_t length, int level, bool flip_y) {
        SDL_Surface *loaded_surface = IMG_Load_RW(SDL_RWFromConstMem(data, length), true);
        SDL_Surface *surface = SDL_ConvertSurfaceFormat(loaded_surface, SDL_PIXELFORMAT_RGBA32, 0);

        if (!flip_y)
            for (size_t y = 0; y < this->h; ++y)
                this->store((uint8_t *)surface->pixels + (this->h - 1 - y) * surface->pitch, surface->w, 1, 1, 0, y,
                            level);
        else
            this->store((uint8_t *)surface->pixels, surface->w, surface->h, 1, 0, 0, level);

        SDL_FreeSurface(loaded_surface);
        SDL_FreeSurface(surface);
    }

    void Texture::store(const uint8_t data[], int width, int height, int offset_x, int offset_y) {
        glTextureSubImage2D(this->id, 0, offset_x, offset_y, width, height, GL_RGBA, GL_UNSIGNED_BYTE, data);
    }

    void Texture::store(const uint8_t data[], int width, int height, int depth, int offset_x, int offset_y,
                        int offset_z) {
        glTextureSubImage3D(this->id, 0, offset_x, offset_y, offset_z, width, height, depth, GL_RGBA, GL_UNSIGNED_BYTE,
                            data);
    }

    void Texture::alloc(unsigned int width, unsigned int height, Format format) {
        this->w = width;
        this->h = height;
        this->d = 1;
        glTextureStorage2D(this->id, 1, format.to_opengl(), this->w, this->h);
    }

    void Texture::alloc(unsigned int width, unsigned int height, unsigned int depth, Format format) {
        this->w = width;
        this->h = height;
        this->d = depth;
        glTextureStorage3D(this->id, 1, format.to_opengl(), this->w, this->h, this->d);
    }

    void Texture::set_wrap_x(Wrap wrap) {
        glTextureParameteri(this->id, GL_TEXTURE_WRAP_S, wrap);
    }

    void Texture::set_wrap_y(Wrap wrap) {
        glTextureParameteri(this->id, GL_TEXTURE_WRAP_T, wrap);
    }

    void Texture::set_mag_filter(Filter filter) {
        glTextureParameteri(this->id, GL_TEXTURE_MAG_FILTER, filter);
    }

    void Texture::set_min_filter(Filter filter) {
        glTextureParameteri(this->id, GL_TEXTURE_MIN_FILTER, filter);
    }

    void Texture::generate_minmap() {
        glGenerateTextureMipmap(this->id);
    }

    Renderbuffer::Renderbuffer(const Format &format, int x, int y) {
        this->create();
        this->alloc(format, x, y);
    }

    void Renderbuffer::create() {
        this->destroy();
        glCreateRenderbuffers(1, &this->id);
    }

    void Renderbuffer::destroy() {
        if (this->id)
            glDeleteRenderbuffers(1, &this->id);
    }

    void Renderbuffer::alloc(const Format &format, int x, int y) {
        glNamedRenderbufferStorage(this->id, format.to_opengl(), x, y);
    }

    void Framebuffer::create() {
        this->destroy();
        glCreateFramebuffers(1, &this->id);
    }

    void Framebuffer::destroy() {
        if (this->id)
            glDeleteFramebuffers(1, &this->id);
    }

    void Framebuffer::bind() {
        glBindFramebuffer(GL_FRAMEBUFFER, this->id);
    }

    void Framebuffer::unbind() {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void Framebuffer::attach(Attachment attachment, Renderbuffer &buffer) {
        glNamedFramebufferRenderbuffer(this->id, attachment, GL_RENDERBUFFER, buffer.get());
    }

    void Framebuffer::attach(unsigned int attachment, Renderbuffer &buffer) {
        glNamedFramebufferRenderbuffer(this->id, COLOR + attachment, GL_RENDERBUFFER, buffer.get());
    }

    void Framebuffer::attach(Attachment attachment, Texture &buffer) {
        glNamedFramebufferTexture(this->id, attachment, buffer.get(), 0);
    }

    void Framebuffer::attach(unsigned int attachment, Texture &buffer) {
        glNamedFramebufferTexture(this->id, COLOR + attachment, buffer.get(), 0);
    }

}
