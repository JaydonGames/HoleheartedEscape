#pragma once
#include <cstddef>
#include <cstdint>
#include <stdexcept>

namespace OpenGL {
    typedef unsigned int id_t;
    
    int max_textures();


    class InitError : public std::runtime_error {
    public:
        InitError(const std::string& what);
        InitError(const char* what);
        InitError(const unsigned char* what);
    };


    class Context {
    public:
        /* The window must be created with SDL_WINDOW_OPENGL and
         * not already have an attached OpenGL context */
        Context(struct SDL_Window* window, int major = 4, int minor = 2, bool core = true);
        void bind();

        /* Returns false if vsync isn't supported */
        static bool enable_vsync();
        /* Needs to be ran when the window resizes. */
        static void set_canvas_size(int x, int y);
        static void clear();
        static void set_clear_color(float r, float g, float b, float a = 1.0f);
        static void sync();
        void swap_buffer();

    private:
        void* context;
        SDL_Window* window;
    };


    class Texture {
    public:
        Texture();
        Texture(const uint8_t data[], size_t length);
        Texture(const Texture&) = delete;
        Texture(Texture&&);
        ~Texture();

        void load(const uint8_t data[], size_t length);
        id_t release();

        void bind(unsigned int index = 0);

        inline unsigned int width() const{
            return this->w;
        }

        inline unsigned int height() const{
            return this->h;
        }
           
        operator bool() const;
        bool operator !() const;

    protected:
        id_t id;

    private:
        int w, h;
    };


    class Buffer {
    public:
        enum Type : unsigned int {
            /* Values from OpenGL */
            vertex = 0x8892,
            index = 0x8893,
            shader = 0x90D2,
            uniform = 0x8A11
        };

        Buffer();
        Buffer(const Buffer&) = delete;
        Buffer(Buffer&&);
        ~Buffer();
        id_t release();

        void bind(Type type);
        void bind(Type type, unsigned int index); /* Implement with glBindBufferBase */

        static void fill(Type type, const void* arr, size_t size, bool dynamic = true);

    protected:
        id_t id;
    };



    class VertexArray {
    public:
        VertexArray(Buffer&& vbo = Buffer{}, Buffer&& ebo = Buffer{});
        VertexArray(const VertexArray&) = delete;
        VertexArray(VertexArray&&);
        ~VertexArray();
        id_t release();

        void bind();
        void draw_triangles(int vertex_count);

        template<typename T>
        void vert_attr(unsigned int index, uintptr_t offset = 0, int stride = sizeof(T));

    protected:
        id_t id;
        Buffer vbo, ebo;
    };



    class Uniform {
    public:
        void store(float);
        void store(float, float);
        void store(float, float, float);
        void store(float, float, float, float);
        void store(unsigned int);
        void store(unsigned int, unsigned int);
        void store(unsigned int, unsigned int, unsigned int);
        void store(unsigned int, unsigned int, unsigned int, unsigned int);
        void store(int);
        void store(int, int);
        void store(int, int, int);
        void store(int, int, int, int);

        template<size_t vec_size>
        void store(const float *arr, size_t count);
        template<size_t vec_size>
        void store(const unsigned int *arr, size_t count);
        template<size_t vec_size>
        void store(const int *arr, size_t count);

    protected:
        int id;
        Uniform(int id) : id(id){}
        friend class Program;
    };



    class Program {
    public:
        Program();
        Program(const char* vert, const char* frag);
        Program(const Program&) = delete;
        Program(Program&&);
        id_t release();
        void load(const char* vert, const char* frag);

        void bind();
        Uniform get_uniform(const char* name);

        operator bool() const;
        bool operator !() const;

    protected:
        id_t id;
    };
}

