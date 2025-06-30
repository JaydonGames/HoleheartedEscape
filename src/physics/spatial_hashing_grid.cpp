
#include "spatial_hashing_grid.hpp"
#include <algorithm>
#include <cmath>
#include <map>
#include <shared_mutex>
#include <string>
#include <unordered_set>
#include <utility>
#include <vector>
#include "graphics/types.hpp"
#include "square.hpp"
#include <iostream>

SpatialHashingGrid::SpatialHashingGrid(Math::Vec2<float> topleft_canvas, Math::Vec2<float> bottomright_canvas,
                                       float tile_width, float tile_height)
    : m_topleft_canvas(topleft_canvas),
      m_bottomright_canvas(bottomright_canvas),
      m_tile_width(tile_width),
      m_tile_height(tile_height) {}

void SpatialHashingGrid::add_object(Square* square) {
    float max_length = square->side_length;
    Math::Vec2<float> pos = square->get_center_position();

    Math::Vec2<float> topleft = get_cell_pos(Math::Vec2<float>(pos.x - max_length / 2, pos.y - max_length / 2));
    Math::Vec2<float> bottomright = get_cell_pos(Math::Vec2<float>(pos.x + max_length / 2, pos.y + max_length / 2));
    object_indexes.insert({square, std::make_pair(topleft, bottomright)});

    for (int r = (int)topleft.x; r < (int)bottomright.x; ++r) {
        for (int c = (int)topleft.y; c < (int)bottomright.y; ++c) {
            Math::Vec2<float> indexes{float(r), float(c)};

            std::string k = get_key(indexes);
            if (grid.count(k)) {
                grid[k].insert(square);
            } else {
                std::unordered_set<Square*> objects = {square};
                grid.insert({k, objects});
            }
        }
    }
}

std::vector<Square*> SpatialHashingGrid::get_closest_objects(Square* square) {
    auto [obj_topleft, obj_bottomright] = object_indexes[square];

    // Math::Vec2<float> extended_topleft = (obj_topleft - 1.0f, obj_topleft.y - 1.0f);
    // Math::Vec2<float> extended_bottomright = (obj_topleft + 1.0f, obj_topleft.y + 1.0f);

    std::vector<Square*> closest_objects;
    for (int r = (int)obj_topleft.x - 1.0f; r <= (int)obj_bottomright.x + 1.0f; ++r) {
        for (int c = (int)obj_topleft.y - 1.0f; c <= (int)obj_bottomright.y + 1.0f; ++c) {
            Math::Vec2<float> indexes{float(r), float(c)};

            std::string k = get_key(indexes);
            for (Square* object : grid[k]) {
                if (object != square) {
                    closest_objects.push_back(object);
                }
            }
        }
    }
    return closest_objects;
}

Math::Vec2<float> SpatialHashingGrid::get_cell_pos(Math::Vec2<float> pos) {
    float ix = pos.x / m_tile_width;
    float iy = pos.y / m_tile_height;

    ix = std::min(std::max(ix, m_topleft_canvas.x), m_bottomright_canvas.x);
    iy = std::min(std::max(iy, m_topleft_canvas.y), m_bottomright_canvas.y);

    return Math::Vec2<float>(ix, iy);
}

std::string SpatialHashingGrid::get_key(Math::Vec2<float> indexes) {
    return std::to_string((int)indexes.x) + ',' + std::to_string((int)indexes.y);
}

void SpatialHashingGrid::remove_object(Square* square) {
    Math::Vec2<float> topleft = object_indexes[square].first;
    Math::Vec2<float> bottomright = object_indexes[square].second;

    for (int r = (int)topleft.x; r < (int)bottomright.x; ++r) {
        for (int c = (int)topleft.y; c < (int)bottomright.y; ++c) {
            Math::Vec2<float> indexes{float(r), float(c)};
            std::string k = get_key(indexes);
            grid[k].erase(square);
        }
    }

    object_indexes.erase(square);
}

void SpatialHashingGrid::update_object(Square* square) {
    remove_object(square);
    add_object(square);
}
