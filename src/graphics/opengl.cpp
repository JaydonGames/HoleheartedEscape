#include "graphics/opengl.hpp"
#include "assets.hpp"
#include <GL/glew.h>
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_surface.h>
#include <SDL_video.h>
#include <cassert>

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

    Context::Context(SDL_Window *window, int major, int minor, bool core)
        : window(window) {
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

    Texture::Texture(const uint8_t data[], size_t length) {
        this->load(data, length);
    }

    Texture::Texture(const Textures::asset_t &texture)
        : Texture(texture.data, texture.length) {}

    Texture::Texture(Texture &&other) {
        this->w = other.w;
        this->h = other.h;
    }

    void Texture::destroy() {
        if (this->id)
            glDeleteTextures(1, &this->id);
        this->id = 0;
    }

    void Texture::load(const uint8_t data[], size_t length) {
        assert(!this->id);
        glGenTextures(1, &this->id);
        this->bind();

        SDL_Surface *loaded_surface = IMG_Load_RW(SDL_RWFromConstMem(data, length), true);
        SDL_Surface *surface = SDL_ConvertSurfaceFormat(loaded_surface, SDL_PIXELFORMAT_RGBA32, 0);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, surface->w, surface->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, surface->pixels);
        this->w = surface->w;
        this->h = surface->h;
        SDL_FreeSurface(loaded_surface);
        SDL_FreeSurface(surface);
        glGenerateMipmap(GL_TEXTURE_2D);
    }

    void Texture::bind(unsigned int index) {
        glActiveTexture(GL_TEXTURE0 + index);
        glBindTexture(GL_TEXTURE_2D, this->id);
    }

    Buffer::Buffer() {
        glGenBuffers(1, &this->id);
    }

    void Buffer::destroy() {
        if (this->id)
            glDeleteBuffers(1, &this->id);
        this->id = 0;
    }

    void Buffer::bind(Type type) {
        glBindBuffer(type, this->id);
    }

    void Buffer::bind(Type type, unsigned int index) {
        glBindBufferBase(type, index, this->id);
    }

    void Buffer::store(Type type, const void *arr, size_t size, bool dynamic) {
        glBufferData(type, size, arr, dynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW);
    }

    void Buffer::allocate(Type type, size_t size) {
        Buffer::store(type, nullptr, size, true);
    }

    void Buffer::sync(Type type) {
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

    void *Buffer::map(Type type, bool writeable) {
        return glMapBuffer(type, writeable ? GL_READ_WRITE : GL_READ_ONLY);
    }

    void Buffer::unmap(Type type) {
        glUnmapBuffer(type);
    }

    VertexArray::VertexArray(Buffer &&vbo, Buffer &&ebo)
        : vbo(std::move(vbo)),
          ebo(std::move(ebo)) {
        glGenVertexArrays(1, &this->id);
        this->bind();
        this->vbo.bind(Buffer::Type::vertex);
        this->ebo.bind(Buffer::Type::index);
    }

    void VertexArray::destroy() {
        if (this->id)
            glDeleteVertexArrays(1, &this->id);
        this->id = 0;
    }

    void VertexArray::bind() {
        glBindVertexArray(this->id);
    }

    void VertexArray::draw_triangles(int vertex_count) {
        glDrawElements(GL_TRIANGLES, vertex_count, GL_UNSIGNED_INT, nullptr);
    }

    template<typename T>
    void VertexArray::vert_attr(unsigned int index, uintptr_t offset, int stride) {
        constexpr int dimensions = std::extent_v<T>;
        using attr_t = std::remove_cvref_t<decltype(std::declval<T>()[0])>;

        unsigned int type;
        if constexpr (std::is_same_v<attr_t, float>)
            type = GL_FLOAT;
        if constexpr (std::is_same_v<attr_t, unsigned int>)
            type = GL_UNSIGNED_INT;
        if constexpr (std::is_same_v<attr_t, int>)
            type = GL_INT;

        glVertexAttribPointer(index, dimensions, type, false, stride, (void *)(offset));
        glEnableVertexAttribArray(index);
    }

    template void VertexArray::vert_attr<float[1]>(unsigned int, uintptr_t, int);
    template void VertexArray::vert_attr<float[2]>(unsigned int, uintptr_t, int);
    template void VertexArray::vert_attr<float[3]>(unsigned int, uintptr_t, int);
    template void VertexArray::vert_attr<float[4]>(unsigned int, uintptr_t, int);
    template void VertexArray::vert_attr<int[1]>(unsigned int, uintptr_t, int);
    template void VertexArray::vert_attr<int[2]>(unsigned int, uintptr_t, int);
    template void VertexArray::vert_attr<int[3]>(unsigned int, uintptr_t, int);
    template void VertexArray::vert_attr<int[4]>(unsigned int, uintptr_t, int);
    template void VertexArray::vert_attr<unsigned int[1]>(unsigned int, uintptr_t, int);
    template void VertexArray::vert_attr<unsigned int[2]>(unsigned int, uintptr_t, int);
    template void VertexArray::vert_attr<unsigned int[3]>(unsigned int, uintptr_t, int);
    template void VertexArray::vert_attr<unsigned int[4]>(unsigned int, uintptr_t, int);

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
    template void Uniform::store<4>(const unsigned int *arr, size_t count);
    template void Uniform::store<1>(const int *arr, size_t count);
    template void Uniform::store<2>(const int *arr, size_t count);
    template void Uniform::store<3>(const int *arr, size_t count);
    template void Uniform::store<4>(const int *arr, size_t count);

    Shader::Shader(Type type, const char *shader) {
        this->compile(type, shader);
    }

    void Shader::compile(Type type, const char *shader) {
        this->id = glCreateShader(type);
        glShaderSource(this->id, 1, &shader, nullptr);
        glCompileShader(id);
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

    void Program::attach(const Shader &shader) {
        if (!this->id)
            this->id = glCreateProgram();
        glAttachShader(this->id, shader.id);
    }

    void Program::destroy() {
        if (this->id)
            glDeleteProgram(this->id);
        this->id = 0;
    }

    void Program::link() {
        glLinkProgram(this->id);
    }

    void Program::bind() {
        glUseProgram(this->id);
    }

    Uniform Program::get_uniform(const char *name) {
        return glGetUniformLocation(this->id, name);
    }

    void Program::bind_uniform_buffer(const char *name, unsigned int binding) {
        glUniformBlockBinding(this->id, glGetUniformBlockIndex(this->id, name), binding);
    }

    void Program::compute(unsigned int x, unsigned int y, unsigned int z) {
        glDispatchCompute(x, y, z);
    }
}
