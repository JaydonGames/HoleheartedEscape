#include <cassert>
#include <GL/glew.h>
#include <SDL.h>
#include <SDL_video.h>
#include <SDL_surface.h>
#include <SDL_image.h>
#include "graphics/opengl.hpp"

namespace OpenGL {

    int max_textures(){
        int max;
        glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &max);
        return max;
    }


    InitError::InitError(const std::string& what)
        : std::runtime_error(what){
    }

    InitError::InitError(const char* what)
        : std::runtime_error(what){
    }

    InitError::InitError(const unsigned char* what)
        : std::runtime_error(std::string(what, what + strlen(reinterpret_cast<const char*>(what)))){
    }



    Context::Context(SDL_Window* window, int major, int minor, bool core) : window(window) {
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, major);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, minor);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, core ? SDL_GL_CONTEXT_PROFILE_CORE : SDL_GL_CONTEXT_PROFILE_COMPATIBILITY);
        this->context = SDL_GL_CreateContext(window);
        if (!this->context)
            throw InitError{SDL_GetError()};
        ::glewExperimental = true;
        unsigned int err = glewInit();
        if (err)
            throw InitError{glewGetErrorString(err)};
    }

    bool Context::enable_vsync(){
        return SDL_GL_SetSwapInterval(1) >= 0;
    }

    void Context::set_canvas_size(int x, int y){
        glViewport(0, 0, x, y);
    }

    void Context::clear(){
        glClear(GL_COLOR_BUFFER_BIT);
    }

    void Context::set_clear_color(float r, float g, float b, float a){
        glClearColor(r, g, b, a);
    }

    void Context::sync(){
        glFinish();
    }

    void Context::swap_buffer(){
        SDL_GL_SwapWindow(this->window);
    }




    Texture::Texture(){
        this->id = 0;
    }

    Texture::Texture(const uint8_t data[], size_t length){
        this->id = 0;
        this->load(data, length);
    }

    Texture::Texture(Texture&& other){
        this->id = other.release();
    }

    Texture::~Texture(){
        if (this->id)
            glDeleteTextures(1, &this->id);
    }

    void Texture::load(const uint8_t data[], size_t length){
        assert(!this->id);
        glGenTextures(1, &this->id);
        this->bind();

        SDL_Surface* loaded_surface = IMG_Load_RW(SDL_RWFromConstMem(data, length), true);
        SDL_Surface* surface = SDL_ConvertSurfaceFormat(loaded_surface, SDL_PIXELFORMAT_RGBA32, 0);


        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, surface->w, surface->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, surface->pixels);
        this->w = surface->w;
        this->h = surface->h;
        SDL_FreeSurface(loaded_surface);
        SDL_FreeSurface(surface);
        glGenerateMipmap(GL_TEXTURE_2D);
    }

    id_t Texture::release(){
        id_t id = this->id;
        this->id = 0;
        return id;
    }

    void Texture::bind(unsigned int index){
        glActiveTexture(GL_TEXTURE0 + index);
        glBindTexture(GL_TEXTURE_2D, this->id);
    }

    Texture::operator bool() const{
        return this->id;
    }

    bool Texture::operator !() const{
        return !this->id;
    }



    Buffer::Buffer(){
        glGenBuffers(1, &this->id);
    }

    Buffer::Buffer(Buffer&& other){
        this->id = other.release();
    }

    Buffer::~Buffer(){
        if (this->id)
            glDeleteBuffers(1, &this->id);
    }

    id_t Buffer::release(){
        id_t id = this->id;
        this->id = 0;
        return id;
    };

    void Buffer::bind(Type type){
        glBindBuffer(type, this->id);
    }

    void Buffer::bind(Type type, unsigned int index){
        glBindBufferBase(type, index, this->id);
    }

    void Buffer::fill(Type type, const void* arr, size_t size, bool dynamic){
        glBufferData(type, size, arr, dynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW);
    }



    VertexArray::VertexArray(Buffer&& vbo, Buffer&& ebo) : vbo(std::move(vbo)), ebo(std::move(ebo)){
        glGenVertexArrays(1, &this->id);
        this->bind();
        this->vbo.bind(Buffer::Type::vertex);
        this->ebo.bind(Buffer::Type::index);
    }

    VertexArray::VertexArray(VertexArray&& other) : vbo(std::move(other.vbo)), ebo(std::move(other.ebo)){
        this->id = other.release();
    }

    VertexArray::~VertexArray(){
        if (this->id)
            glDeleteVertexArrays(1, &this->id);
    }

    id_t VertexArray::release(){
        id_t id = this->id;
        this->id = 0;
        return id;
    }

    void VertexArray::bind(){
        glBindVertexArray(this->id);
    }

    void VertexArray::draw_triangles(int vertex_count){
        glDrawElements(GL_TRIANGLES, vertex_count, GL_UNSIGNED_INT, nullptr);
    }

    template<typename T>
    void VertexArray::vert_attr(unsigned int index, uintptr_t offset, int stride){
        constexpr int dimensions = std::extent_v<T>;
        using attr_t = std::remove_cvref_t<decltype(std::declval<T>()[0])>;

        unsigned int type;
        if constexpr (std::is_same_v<attr_t, float>)
            type = GL_FLOAT;
        if constexpr (std::is_same_v<attr_t, unsigned int>)
            type = GL_UNSIGNED_INT;
        if constexpr (std::is_same_v<attr_t, int>)
            type = GL_INT;

        glVertexAttribPointer(index, dimensions, type, false, stride, (void*)(offset));
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



    void Uniform::store(float f1){
        glUniform1f(this->id, f1);
    }

    void Uniform::store(float f1, float f2){
        glUniform2f(this->id, f1, f2);
    }

    void Uniform::store(float f1, float f2, float f3){
        glUniform3f(this->id, f1, f2, f3);
    }

    void Uniform::store(float f1, float f2, float f3, float f4){
        glUniform4f(this->id, f1, f2, f3, f4);
    }

    void Uniform::store(unsigned int u1){
        glUniform1ui(this->id, u1);
    }

    void Uniform::store(unsigned int u1, unsigned int u2){
        glUniform2ui(this->id, u1, u2);
    }

    void Uniform::store(unsigned int u1, unsigned int u2, unsigned int u3){
        glUniform3ui(this->id, u1, u2, u3);
    }

    void Uniform::store(unsigned int u1, unsigned int u2, unsigned int u3, unsigned int u4){
        glUniform4ui(this->id, u1, u2, u3, u4);
    }

    void Uniform::store(int i1){
        glUniform1i(this->id, i1);
    }

    void Uniform::store(int i1, int i2){
        glUniform2i(this->id, i1, i2);
    }

    void Uniform::store(int i1, int i2, int i3){
        glUniform3i(this->id, i1, i2, i3);
    }

    void Uniform::store(int i1, int i2, int i3, int i4){
        glUniform4i(this->id, i1, i2, i3, i4);
    }

    template<size_t vec_size>
    void Uniform::store(const float* arr, size_t count){
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
    void Uniform::store(const unsigned int* arr, size_t count){
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
    void Uniform::store(const int* arr, size_t count){
        if constexpr (vec_size == 1)
            glUniform1iv(this->id, count, arr);
        if constexpr (vec_size == 2)
            glUniform2iv(this->id, count, arr);
        if constexpr (vec_size == 3)
            glUniform3iv(this->id, count, arr);
        if constexpr (vec_size == 4)
            glUniform4iv(this->id, count, arr);
    }

    template void Uniform::store<1>(const float* arr, size_t count);
    template void Uniform::store<2>(const float* arr, size_t count);
    template void Uniform::store<3>(const float* arr, size_t count);
    template void Uniform::store<4>(const float* arr, size_t count);
    template void Uniform::store<1>(const unsigned int* arr, size_t count);
    template void Uniform::store<2>(const unsigned int* arr, size_t count);
    template void Uniform::store<3>(const unsigned int* arr, size_t count);
    template void Uniform::store<4>(const unsigned int* arr, size_t count);
    template void Uniform::store<1>(const int* arr, size_t count);
    template void Uniform::store<2>(const int* arr, size_t count);
    template void Uniform::store<3>(const int* arr, size_t count);
    template void Uniform::store<4>(const int* arr, size_t count);



    Program::Program(){
        this->id = 0;
    }

    Program::Program(const char* vert, const char* frag){
        this->load(vert, frag);
    }

    Program::Program(Program&& other){
        this->id = other.release();
    }

    id_t Program::release(){
        id_t id = this->id;
        this->id = 0;
        return id;
    }

    inline unsigned int compile_shader(const char* shader, unsigned int type){
        unsigned int id = glCreateShader(type);
        glShaderSource(id, 1, &shader, nullptr);
        glCompileShader(id);
        int status;
        glGetShaderiv(id, GL_COMPILE_STATUS, &status);
        if (!status) {
            char log[512];
            glGetShaderInfoLog(id, 512, NULL, log);
            throw InitError{log};
        }

        return id;
    }

    void Program::load(const char* vert, const char* frag){
        this->id = glCreateProgram();
        unsigned int vert_shader = compile_shader(vert, GL_VERTEX_SHADER);
        unsigned int frag_shader = compile_shader(frag, GL_FRAGMENT_SHADER);
        glAttachShader(this->id, vert_shader);
        glAttachShader(this->id, frag_shader);
        glLinkProgram(this->id);
        glDeleteShader(vert_shader);
        glDeleteShader(frag_shader);
    }

    void Program::bind(){
        glUseProgram(this->id);
    }

    Uniform Program::get_uniform(const char* name){
        return glGetUniformLocation(this->id, name);
    }

    void Program::bind_uniform_buffer(const char* name, unsigned int binding){
        glUniformBlockBinding(this->id, glGetUniformBlockIndex(this->id, name), binding);
    }

    Program::operator bool() const{
        return this->id;
    }

    bool Program::operator !() const{
        return !this->id;
    }
}
