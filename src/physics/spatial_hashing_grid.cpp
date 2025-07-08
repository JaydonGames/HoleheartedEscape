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
#include "physics/shape.hpp"
#include <iostream>

SpatialHashingGrid::SpatialHashingGrid(Math::Vec2<float> topleft_canvas, Math::Vec2<float> bottomright_canvas,
                                       float tile_width, float tile_height)
    : m_topleft_canvas(topleft_canvas),
      m_bottomright_canvas(bottomright_canvas),
      m_tile_width(tile_width),
      m_tile_height(tile_height) {}

void SpatialHashingGrid::add_object(Shape* object) {
    float max_length = object->get_max_side_length();
    Math::Vec2<float> pos = object->get_center_position();

    Math::Vec2<float> topleft = get_cell_pos(Math::Vec2<float>(pos.x - max_length / 2, pos.y - max_length / 2));
    Math::Vec2<float> bottomright = get_cell_pos(Math::Vec2<float>(pos.x + max_length / 2, pos.y + max_length / 2));
    object_indexes.insert({object, std::make_pair(topleft, bottomright)});

    for (int r = (int)topleft.x; r < (int)bottomright.x; ++r) {
        for (int c = (int)topleft.y; c < (int)bottomright.y; ++c) {
            Math::Vec2<float> indexes{float(r), float(c)};

            std::string k = get_key(indexes);
            if (grid.count(k)) {
                grid[k].insert(object);
            } else {
                std::unordered_set<Shape*> objects = {object};
                grid.insert({k, objects});
            }
        }
    }
}

std::vector<Shape*> SpatialHashingGrid::get_closest_objects(Shape* object) {
    auto [obj_topleft, obj_bottomright] = object_indexes[object];

    std::vector<Shape*> closest_objects;
    for (int r = (int)obj_topleft.x - 1.0f; r <= (int)obj_bottomright.x + 1.0f; ++r) {
        for (int c = (int)obj_topleft.y - 1.0f; c <= (int)obj_bottomright.y + 1.0f; ++c) {
            Math::Vec2<float> indexes{float(r), float(c)};

            std::string k = get_key(indexes);
            for (Shape* other_object : grid[k]) {
                if (object != other_object) {
                    closest_objects.push_back(other_object);
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

void SpatialHashingGrid::remove_object(Shape* object) {
    Math::Vec2<float> topleft = object_indexes[object].first;
    Math::Vec2<float> bottomright = object_indexes[object].second;

    for (int r = (int)topleft.x; r < (int)bottomright.x; ++r) {
        for (int c = (int)topleft.y; c < (int)bottomright.y; ++c) {
            Math::Vec2<float> indexes{float(r), float(c)};
            std::string k = get_key(indexes);
            grid[k].erase(object);
        }
    }

    object_indexes.erase(object);
}

void SpatialHashingGrid::update_object(Shape* object) {
    remove_object(object);
    add_object(object);
}
