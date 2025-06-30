#pragma once

#include <map>
#include <string>
#include <unordered_set>
#include <utility>
#include <vector>
#include "graphics/types.hpp"
#include "square.hpp"

// FIX: Math::Vec2 can hold ints or floats. Change that when deemed better
class SpatialHashingGrid {
    float m_tile_width;
    float m_tile_height;
    Math::Vec2<float> m_topleft_canvas;
    Math::Vec2<float> m_bottomright_canvas;

public:
    std::map<Square*, std::pair<Math::Vec2<float>, Math::Vec2<float>>> object_indexes;
    // NOTE: Should we use unordered or ordered set?
    // Can we combine object_indexes here? I tried but I got template errors.
    std::map<std::string, std::unordered_set<Square*>> grid;

    // FIX: Aydon said that the tile size was 64??? Even tho when you count the num tiles and divide by dims you get
    // tile size of 64
    SpatialHashingGrid(Math::Vec2<float> topleft_canvas, Math::Vec2<float> bottomright_canvas, float tile_width = 16.0f,
                       float tile_height = 16.0f);

    void add_object(Square* square);

    std::vector<Square*> get_closest_objects(Square* square);

    Math::Vec2<float> get_cell_pos(Math::Vec2<float> pos);

    std::string get_key(Math::Vec2<float> indexes);

    void remove_object(Square* square);

    void update_object(Square* square);
};
