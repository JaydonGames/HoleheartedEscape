#pragma once
#include <cstddef>
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>
#include "graphics/types.hpp"
#include "grid.hpp"

namespace Tiled {

    enum class RenderOrder {
        RightDown,
        RightUp,
        LeftDown,
        LeftUp
    };

    struct Tile {
        Math::Rect coords;
        size_t tex;
    };

    struct MapTile {
        Tile* tile = nullptr;
        unsigned int flags = 0;

    private:
        friend struct Map;
        static MapTile from_gid(uint32_t gid, std::vector<struct TilesetId>);
    };

    struct Layer {
        const char* name;
        size_t width, height;
        Grid<MapTile> tiles;
    };

    struct Map {
        Map(class World& world, const char* map);

        RenderOrder render_order;
        size_t width, height;
        size_t tile_width, tile_height;
        std::vector<Layer> layers;

        Layer& operator[](std::string_view str);
        Layer& operator[](size_t i);
    };

    struct Tileset {
        Tileset() = default;
        Tileset(World* world, const char* tileset);
        std::vector<Tile> tiles;
    };

    class World {
    public:
        void register_texture(const char* name, size_t tex);
        void register_tileset(const char* name, const char* tileset);
        void register_tileset(const char* name, Tileset&& tileset);
        void register_tileset(const char* name, const Tileset& tileset);
        size_t get_texture(std::string_view name);
        Tileset& get_tileset(std::string_view name);

    private:
        std::unordered_map<std::string_view, size_t> textures;
        std::unordered_map<std::string_view, Tileset> tilesets;
    };

}
