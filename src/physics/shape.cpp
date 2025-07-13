#include "shape.hpp"

Shape::Shape(bool is_static, float mass, bool is_player)
    : is_static(is_static),
      is_player(is_player),
      mass(mass) {}
