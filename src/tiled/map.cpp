#include <cstdint>
#include <sstream>
#include <string_view>
#include <filesystem>
#include "tiled/map.hpp"
#include "graphics/render.hpp"
#include "tinyxml2.h"

namespace fs = std::filesystem;

namespace Tiled {

    struct TilesetId {
        uint32_t id;
        Tileset* tileset;
    };

    MapTile MapTile::from_gid(uint32_t gid, std::vector<TilesetId> tilesets) {
        if (!gid)
            return {};

        MapTile tile;
        tile.flags |= gid & (1 << 31) ? Render::Flags::FlipX : 0;
        tile.flags |= gid & (1 << 30) ? Render::Flags::FlipY : 0;
        tile.flags |= gid & (1 << 29) ? Render::Flags::FlipDiag : 0;
        gid &= 0xfffffff;

        size_t tileset;
        for (tileset = 0; tileset < tilesets.size() - 1; ++tileset)
            if (tilesets[tileset + 1].id > gid)
                break;

        tile.tile = &tilesets[tileset].tileset->tiles[gid - tilesets[tileset].id];
        return tile;
    }

    Map::Map(World& world, const char* map) {
        tinyxml2::XMLDocument doc;
        tinyxml2::XMLElement* root;
        doc.Parse(map);
        root = doc.RootElement();

        const char* render_order = "";
        root->QueryAttribute("renderorder", &render_order);
        root->QueryAttribute("width", &this->width);
        root->QueryAttribute("height", &this->height);
        root->QueryAttribute("tilewidth", &this->tile_width);
        root->QueryAttribute("tileheight", &this->tile_height);

        if (render_order == std::string_view{"right-up"})
            this->render_order = RenderOrder::RightUp;
        else if (render_order == std::string_view{"left-down"})
            this->render_order = RenderOrder::LeftDown;
        else if (render_order == std::string_view{"left-up"})
            this->render_order = RenderOrder::LeftUp;
        else
            this->render_order = RenderOrder::RightDown;

        const char* name;
        std::vector<TilesetId> tilesets;
        for (tinyxml2::XMLElement* tileset = root->FirstChildElement("tileset"); tileset;
             tileset = tileset->NextSiblingElement("tileset")) {
            TilesetId& entry = tilesets.emplace_back();
            tileset->QueryAttribute("firstgid", &entry.id);
            tileset->QueryAttribute("source", &name);
            entry.tileset = &world.get_tileset(name);
        }

        for (tinyxml2::XMLElement* layer = root->FirstChildElement("layer"); layer;
             layer = layer->NextSiblingElement("layer")) {
            size_t width, height;
            const char* name;
            layer->QueryAttribute("width", &width);
            layer->QueryAttribute("height", &height);
            layer->QueryAttribute("name", &name);

            tinyxml2::XMLElement* data = layer->FirstChildElement("data");
            const char* encoding = "";
            data->QueryAttribute("encoding", &encoding);

            Grid<MapTile>& grid = this->tile_layers[name] = {width, height};
            if (encoding == std::string_view{"csv"}) {
                std::stringstream csv{data->GetText()};

                uint32_t gid;
                for (size_t y = 0; y < height; ++y) {
                    for (size_t x = 0; x < width; ++x) {
                        csv >> gid;
                        csv.ignore();
                        grid[y][x] = MapTile::from_gid(gid, tilesets);
                    }
                }
            }
        }

        for (tinyxml2::XMLElement* layer = root->FirstChildElement("objectgroup"); layer;
             layer = layer->NextSiblingElement("objectgroup")) {
            const char* name;
            layer->QueryAttribute("name", &name);
            auto& objects = this->object_layers[name];

            for (tinyxml2::XMLElement* object = layer->FirstChildElement("object"); object;
                 object = object->NextSiblingElement("object")) {
                auto& pos = objects[name];
                object->QueryAttribute("x", &pos.x);
                object->QueryAttribute("y", &pos.y);
                object->QueryAttribute("name", &name);
            }
        }

    }

    Tileset::Tileset(World* world, const char* tileset) {
        tinyxml2::XMLDocument doc;
        tinyxml2::XMLElement *root, *image;
        doc.Parse(tileset);
        root = doc.RootElement();
        image = root->FirstChildElement("image");

        const char* image_file;
        int tile_width, tile_height, tile_count, column_count, image_width, image_height;
        root->QueryAttribute("tilewidth", &tile_width);
        root->QueryAttribute("tileheight", &tile_height);
        root->QueryAttribute("tilecount", &tile_count);
        root->QueryAttribute("columns", &column_count);
        image->QueryAttribute("width", &image_width);
        image->QueryAttribute("height", &image_height);
        image->QueryAttribute("source", &image_file);

        for (size_t i = 0; i < tile_count; ++i) {
            int x = i % column_count, y = size_t(i / column_count);
            this->tiles.emplace_back(Math::Rect{x * tile_width, y * tile_height, tile_width, tile_height},
                                     world->get_texture(image_file));
        }
    }

    void World::register_tileset(const char* name, const char* tileset) {
        tilesets[name] = {this, tileset};
    }

    void World::register_tileset(const char* name, Tileset&& tileset) {
        tilesets[name] = std::move(tileset);
    }

    void World::register_tileset(const char* name, const Tileset& tileset) {
        tilesets[name] = tileset;
    }

    void World::register_texture(const char* name, size_t tex) {
        textures[name] = tex;
    }

    size_t World::get_texture(std::string_view name) {
        std::string name_no_ext = fs::path{name}.stem().string();
        return textures[name_no_ext];
    }

    Tileset& World::get_tileset(std::string_view name) {
        std::string name_no_ext = fs::path{name}.stem().string();
        return tilesets[name_no_ext];
    }

};
