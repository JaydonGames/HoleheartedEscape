#pragma once
#include <cstddef>
#include <cstdint>
#include <initializer_list>
#include <stdexcept>
#include <type_traits>

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

    class Context {
    public:
        /* The window must be created with SDL_WINDOW_OPENGL and
         * not already have an attached OpenGL context */
        Context(SDL_Window* window, int major = 4, int minor = 5, bool core = true);
        void bind();

        static void wireframe(bool enable);
        static void blending(bool enable);
        /* Returns false if vsync isn't supported */
        static bool enable_vsync();
        /* Needs to be ran when the window resizes. */
        static void set_canvas_size(int x, int y);
        static void clear();
        static void set_clear_color(float r, float g, float b, float a = 1.0f);
        static void sync();
        void swap_buffer();

        static bool debug;

    private:
        void* context;
        SDL_Window* window;
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
            return *this;
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

        id_t get() {
            return id;
        }

        operator bool() {
            return id;
        }

        bool operator!() {
            return !static_cast<T*>(this)->operator bool();
        }

    protected:
        Entity() {}
        id_t id = 0;
    };

    struct Format {
        enum Components {
            R,
            RG,
            RGB,
            RGBA,
            DEPTH,
            DEPTH_STENCIL
        };

        enum Type {
            Typed,
            Normalized,
        } type;

        union {
            struct {
                bool is_signed;
                bool is_floating_point;
                size_t size;
                Components component;
            } typed;
            struct {
                bool is_signed;
                size_t bits;
                Components component;
            } norm;
        };

        unsigned int to_opengl() const;
        unsigned int to_sdl() const;
    };

    template<size_t size, bool is_signed = false>
    Format format(Format::Components comp) {
        Format fmt{Format::Normalized};
        fmt.norm = {is_signed, size, comp};
        return fmt;
    }

    template<typename T>
    Format format(Format::Components comp) {
        Format fmt{Format::Typed};
        fmt.typed = {std::is_signed_v<T>, std::is_floating_point_v<T>, sizeof(T), comp};
        return fmt;
    }

    class Buffer : public Entity<Buffer> {
    public:
        enum Type : unsigned int {
            vertex = 0x8892,
            index = 0x8893,
            shader = 0x90D2,
            uniform = 0x8A11
        };

        Buffer() {}
        Buffer(Type type);
        Buffer(Type type, const void* arr, size_t size, bool dynamic = true);
        Buffer(Type type, size_t size);

        void create(Type type);
        void destroy();

        void bind(unsigned int index);
        void store(const void* arr, size_t size, bool dynamic = true);
        void alloc(size_t size);

        void sync();

        void get_data(void* data, size_t size, ptrdiff_t offset = 0) const;
        void* map(bool writeable = true);
        void unmap();

    private:
        Type type = (Type)0;
        friend class VertexArray;
    };

    class VertexArray : public Entity<VertexArray> {
    public:
        VertexArray() {}
        VertexArray(ptrdiff_t offset, int stride);

        void create();
        void destroy();
        void bind();

        Buffer& attach_vbo(ptrdiff_t offset, int stride, Buffer&& buffer = Buffer{OpenGL::Buffer::vertex});
        Buffer& attach_ebo(Buffer&& buffer = Buffer{OpenGL::Buffer::index});
        Buffer& get_vbo();
        Buffer& get_ebo();
        template<typename T>
        void vert_attr(unsigned int index, unsigned int offset, bool normalized = false);

    private:
        Buffer vbo, ebo;
    };

    class Uniform {
    public:
        Uniform() {}

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
        int id = 0;
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

        void create(Type type, const char* shader);
        void destroy();

        friend class Program;
    };

    class Program : public Entity<Program> {
    public:
        Program() {}
        Program(std::initializer_list<Shader> shaders);
        Program(std::initializer_list<Shader*> shaders);

        void create();
        void destroy();
        void use();

        void attach(const Shader&);
        void link();

        Uniform get_uniform(const char* name);
        void bind_uniform_block(const char* name, unsigned int binding);

        void compute(unsigned int x = 1, unsigned int y = 1, unsigned int z = 1);
        void draw_tri(VertexArray& vao, int vert_count);
        void draw_patches(VertexArray& vao, int vert_count, int vert_per_patch);
    };

    class Texture : public Entity<Texture> {
    public:
        enum Type : unsigned int {
            tex2d = 0x0DE1,
            tex3d = 0x806F,
            texarr2d = 0x8C1A,
        };

        enum Wrap {
            repeat = 0x2901,
            mirrored_repeat = 0x8370,
            clamp_border = 0x812D,
            clamp_edge = 0x812F,
        };

        enum Filter {
            nearest = 0x2600,
            linear = 0x2601,
            linear_mipmap_linear_pixel = 0x2703,
            linear_mipmap_nearest_pixel = 0x2702,
            nearest_mipmap_linear_pixel = 0x2701,
            nearest_mipmap_nearest_pixel = 0x2700,
        };

        Texture() {}
        Texture(Type type);
        Texture(Type type, const uint8_t data[], size_t length);
        Texture(Type type, const uint8_t data[], size_t length, int level);
        Texture(Type type, unsigned int width, unsigned int height, Format fmt = format<8>(Format::RGBA));
        Texture(Type type, unsigned int width, unsigned int height, unsigned int depth,
                Format fmt = format<8>(Format::RGBA));
        Texture(const Textures::asset_t& texture);
        Texture(Texture&&);

        void create(Type);
        void destroy();

        void bind(unsigned int index);
        void load(const uint8_t data[], size_t length, bool flip_y = false);
        void load(const uint8_t data[], size_t length, int level, bool flip_y);
        void store(const uint8_t data[], int width, int height, int offset_x = 0, int offset_y = 0);
        void store(const uint8_t data[], int width, int height, int depth, int offset_x, int offset_y, int offset_z);
        void alloc(unsigned int width, unsigned int height, Format fmt = format<8>(Format::RGBA));
        void alloc(unsigned int width, unsigned int height, unsigned int depth, Format fmt = format<8>(Format::RGBA));

        void set_wrap_x(Wrap);
        void set_wrap_y(Wrap);
        void set_mag_filter(Filter);
        void set_min_filter(Filter);
        void generate_minmap();

        inline unsigned int width() const {
            return this->w;
        }

        inline unsigned int height() const {
            return this->h;
        }

        inline unsigned int depth() const {
            return this->d;
        }

    private:
        int w, h, d;
        Type type;
    };

    class Renderbuffer : public Entity<Renderbuffer> {
    public:
        Renderbuffer() {}
        Renderbuffer(const Format& format, int x, int y);

        void create();
        void destroy();

        void alloc(const Format& format, int x, int y);
    };

    class Framebuffer : public Entity<Framebuffer> {
    public:
        enum Attachment {
            COLOR = 0x8CE0,
            DEPTH = 0x8D00,
            STENCIL = 0x8D20,
            DEPTH_STENCIL = 0x821A,
        };

        Framebuffer() {};

        void create();
        void destroy();

        void bind();
        void unbind();

        void attach(Attachment attachment, Renderbuffer& buffer);
        void attach(unsigned int color_attachment, Renderbuffer& buffer);
        void attach(Attachment attachment, Texture& buffer);
        void attach(unsigned int color_attachment, Texture& buffer);
    };

}
