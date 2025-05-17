#pragma once
#include <cstddef>
#include <cstdint>
#include <initializer_list>
#include <stdexcept>

struct SDL_Window;

namespace Textures {
    struct asset_t;
}

namespace OpenGL {
    typedef unsigned int id_t;

    int max_textures();

    class InitError : public std::runtime_error {
    public:
        InitError(const std::string& what);
        InitError(const char* what);
        InitError(const unsigned char* what);
    };

    template<typename T>
    class Entity {
    public:
        Entity(const Entity&) = delete;
        Entity& operator=(const Entity&) = delete;

        Entity(Entity&& entity) {
            id = entity.release();
        }

        Entity& operator=(Entity&& entity) {
            id = entity.release();
        }

        ~Entity() {
            if (this->id)
                static_cast<T*>(this)->destroy();
        }

        id_t release() {
            id_t id = this->id;
            this->id = 0;
            return id;
        }

        operator bool() {
            return id;
        }

        bool operator!() {
            return !id;
        }

    protected:
        Entity() {}
        id_t id = 0;
    };

    class Context {
    public:
        /* The window must be created with SDL_WINDOW_OPENGL and
         * not already have an attached OpenGL context */
        Context(SDL_Window* window, int major = 4, int minor = 1, bool core = true);
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

    class Texture : public Entity<Texture> {
    public:
        Texture() {}
        Texture(const uint8_t data[], size_t length);
        Texture(const Textures::asset_t& texture);
        Texture(Texture&&);

        void load(const uint8_t data[], size_t length);
        void destroy();
        void bind(unsigned int index = 0);

        inline unsigned int width() const {
            return this->w;
        }

        inline unsigned int height() const {
            return this->h;
        }

    private:
        int w, h;
    };

    class Buffer : public Entity<Buffer> {
    public:
        enum Type : unsigned int {
            /* Values from OpenGL */
            vertex = 0x8892,
            index = 0x8893,
            shader = 0x90D2,
            uniform = 0x8A11
        };

        Buffer();

        void destroy();
        void bind(Type type);
        void bind(Type type, unsigned int index);

        static void store(Type type, const void* arr, size_t size, bool dynamic = true);
        static void allocate(Type type, size_t size);
        static void* map(Type type, bool writeable = true);
        static void unmap(Type type);
    };

    class VertexArray : public Entity<Buffer> {
    public:
        VertexArray(Buffer&& vbo = Buffer{}, Buffer&& ebo = Buffer{});

        void destroy();
        void bind();
        void draw_triangles(int vertex_count);

        template<typename T>
        void vert_attr(unsigned int index, uintptr_t offset = 0, int stride = sizeof(T));

    protected:
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
        void store(const float* arr, size_t count);
        template<size_t vec_size>
        void store(const unsigned int* arr, size_t count);
        template<size_t vec_size>
        void store(const int* arr, size_t count);

    protected:
        int id;
        Uniform(int id)
            : id(id) {}
        friend class Program;
    };

    class Shader : public Entity<Shader> {
    public:
        enum Type : unsigned int {
            vert = 0x8B31,
            frag = 0x8B30,
            tcs = 0x8E88,
            tes = 0x8E87,
            geo = 0x8DD9,
            comp = 0x91B9
        };

        Shader() {}
        Shader(Type type, const char* shader);
        void compile(Type type, const char* shader);
        void destroy();
        friend class Program;
    };

    class Program : public Entity<Program> {
    public:
        Program() {}
        void attach(const Shader&);
        void link();

        Program(std::initializer_list<Shader> shaders) {
            for (const Shader& shader : shaders)
                this->attach(shader);
            this->link();
        }

        Program(std::initializer_list<std::reference_wrapper<Shader>> shaders) {
            for (Shader& shader : shaders)
                this->attach(shader);
            this->link();
        }

        void destroy();
        void bind();
        Uniform get_uniform(const char* name);
        void bind_uniform_buffer(const char* name, unsigned int binding);
    };

}
