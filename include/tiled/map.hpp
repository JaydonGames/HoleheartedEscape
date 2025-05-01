#pragma once
#include <cstddef>
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>
#include "graphics/render.hpp"
#include "grid.hpp"

namespace Tiled {

    enum class RenderOrder {
        RightDown,
        RightUp,
        LeftDown,
        LeftUp
    };




    class Tile {
    public:
        Tile(){}
        bool empty = true;
        Render::Rect get_coords();
        OpenGL::Texture& get_texture();
        unsigned int get_flags();

    private:
        friend class Map;
        struct Data {
            Render::Rect coords;
            size_t texture;
        };

        Tile(uint32_t gid, const std::vector<std::pair<uint32_t, size_t>>& tileset_gids);
        unsigned int render_flags = 0;
        Data& get_data();
        uint32_t tileset = 0;
        uint32_t tile = 0;
    };

    struct Layer {
        size_t width, height;
        Grid<Tile> tiles;
        std::string name;
    };

    class Map {
    public:
        Map(const char* map);

        RenderOrder render_order;
        size_t width, height;
        size_t tile_width, tile_height;
        std::vector<Layer> layers;

        static void register_tileset(const char* name, const char* tileset);
        static void register_texture(const char* name, OpenGL::Texture&& texture);

    private:
        friend class Tile;
        static std::vector<Tile::Data>& get_tileset(size_t id);
        static OpenGL::Texture& get_texture(size_t id);
        static size_t get_tileset(const std::string name);
        static size_t get_texture(const std::string name);
        static std::vector<OpenGL::Texture> textures;
        static std::vector<std::vector<Tile::Data>> tilesets;
        static std::unordered_map<std::string, size_t> texture_names, tileset_names;

        Grid<Tile> parse_csv_grid(size_t width, size_t height, const char* csv,
                const std::vector<std::pair<uint32_t, size_t>>& tileset_gids);
    };

}
