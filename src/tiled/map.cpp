#include <filesystem>
#include <sstream>
#include <string_view>
#include "map.hpp"
#include "tinyxml2.h"

namespace Tiled {

    Tile::Tile(uint32_t gid, const std::vector<std::pair<uint32_t, size_t>>& tileset_gids){
        if (!gid)
            return;

        empty = false;
        this->render_flags |= gid & (1<<31) ? Render::Flags::FlipX : 0;
        this->render_flags |= gid & (1<<30) ? Render::Flags::FlipY : 0;
        this->render_flags |= gid & (1<<29) ? Render::Flags::FlipDiagonally : 0;
        gid &= 0xfffffff;

        for (this->tileset = 0; tileset+1 < tileset_gids.size(); ++tileset)
            if (tileset_gids[this->tileset+1].first > gid)
                break;

        this->tile = gid - tileset_gids[this->tileset].first;
        this->tileset = tileset_gids[this->tileset].second;
    }
    
    Tile::Data& Tile::get_data(){
        return Tiled::Map::get_tileset(this->tileset)[this->tile];
    }

    Render::Rect Tile::get_coords(){
        return this->get_data().coords;
    }

    unsigned int Tile::get_flags(){
        return this->render_flags;
    }

    OpenGL::Texture& Tile::get_texture(){
        return Map::get_texture(this->get_data().texture);
    }



    RenderOrder render_order_from_str(std::string_view str){
        if (str == "right-up")
            return RenderOrder::RightUp;
        if (str ==  "left-down")
            return RenderOrder::LeftDown;
        if (str == "left-up")
            return RenderOrder::LeftUp;
        return RenderOrder::RightDown;
    }

    Grid<Tile> Map::parse_csv_grid(size_t width, size_t height, const char* csv, const std::vector<std::pair<uint32_t, size_t>>& tileset_gids){
        Grid<Tile> layer{width, height};

        std::stringstream data{csv};
        for (size_t y = 0; y < height; ++y){
            for (size_t x = 0; x < width; ++x){
                uint32_t gid;
                data >> gid;
                data.ignore();
                layer[y][x] = Tile{gid, tileset_gids};
            }
        }

        return layer;
    }

    Map::Map(const char* map_string){
        tinyxml2::XMLDocument doc;
        doc.Parse(map_string);
        tinyxml2::XMLElement* map = doc.RootElement();

        const char* render_order = "right-down";
        map->QueryAttribute("renderorder", &render_order);
        this->render_order = render_order_from_str(render_order);

        map->QueryAttribute("width", &this->width);
        map->QueryAttribute("height", &this->height);
        map->QueryAttribute("tilewidth", &this->tile_width);
        map->QueryAttribute("tileheight", &this->tile_height);

        std::vector<std::pair<uint32_t, size_t>> tileset_gids;
        for (tinyxml2::XMLElement* tileset = map->FirstChildElement("tileset");
                tileset; tileset = tileset->NextSiblingElement("tileset")){
            std::pair<uint32_t, size_t>& entry = tileset_gids.emplace_back();
            tileset->QueryAttribute("firstgid", &entry.first);
            const char* texture;
            tileset->QueryAttribute("source", &texture);
            entry.second = Map::get_texture(texture);
        }

        for (tinyxml2::XMLElement* layer_elem = map->FirstChildElement("layer");
                layer_elem; layer_elem = layer_elem->NextSiblingElement("layer")){
            Layer layer;
            const char* name;
            layer_elem->QueryAttribute("width", &layer.width);
            layer_elem->QueryAttribute("height", &layer.height);
            layer_elem->QueryAttribute("name", &name);
            layer.name = name;

            tinyxml2::XMLElement* data = layer_elem->FirstChildElement("data");
            const char* encoding = "csv";
            data->QueryAttribute("encoding", &encoding);

            if (std::string_view{encoding} == "csv")
                layer.tiles = parse_csv_grid(layer.width, layer.height, data->GetText(), tileset_gids);

            this->layers.push_back(layer);
        }

    }

    std::vector<OpenGL::Texture> Map::textures;
    std::vector<std::vector<Tile::Data>> Map::tilesets;
    std::unordered_map<std::string, size_t> Map::texture_names, Map::tileset_names;

    void Map::register_tileset(const char* filename, const char* tileset_string){
        tinyxml2::XMLDocument doc;
        doc.Parse(tileset_string);
        tinyxml2::XMLElement* tileset = doc.RootElement();
        tinyxml2::XMLElement* image = tileset->FirstChildElement("image");
        int tile_width, tile_height, tile_count, column_count, image_width, image_height;
        const char* image_file;
        tileset->QueryAttribute("tilewidth", &tile_width);
        tileset->QueryAttribute("tileheight", &tile_height);
        tileset->QueryAttribute("tilecount", &tile_count);
        tileset->QueryAttribute("columns", &column_count);
        image->QueryAttribute("width", &image_width);
        image->QueryAttribute("height", &image_height);
        image->QueryAttribute("source", &image_file);

        std::vector<Tile::Data> tiles;
        for (size_t i = 0; i < tile_count; ++i){
            int x = i % column_count, y = i / column_count;
            tiles.emplace_back(Render::Rect{x*tile_width, y*tile_height, tile_width, tile_height}, Map::get_texture(image_file));
        }

        tileset_names[filename] = &Map::tilesets.emplace_back(std::move(tiles)) - Map::tilesets.data();
    }


    void Map::register_texture(const char* name, OpenGL::Texture&& texture){
        Map::texture_names[std::string{name}] = &Map::textures.emplace_back(std::move(texture)) - Map::textures.data();
    }

    OpenGL::Texture& Map::get_texture(size_t id){
        return Map::textures[id];
    }

    size_t Map::get_texture(const std::string name){
        return Map::texture_names[std::filesystem::path{name}.stem()];
    }

    std::vector<Tile::Data>& Map::get_tileset(size_t id){
        return Map::tilesets[id];
    }

    size_t Map::get_tileset(const std::string name){
        return Map::tileset_names[std::filesystem::path{name}.stem()];
    }


}
