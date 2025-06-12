#pragma once
#include <utility>
#include "graphics/types.hpp"
#include "tiled/map.hpp"

class ObjectTest {
public:
    static constexpr int OBJECT_MASS = 100;

    ObjectTest(int x, int y, Tiled::Layer collision_layer);

    Math::Rect get_rect();

    void update(double dt);

    void do_layer_collision(std::string direction);

    double m_vel_x, m_vel_y, m_gravity, m_acceleration;

private:
    Math::Rect m_rect;

    Tiled::Layer collision_layer;

    void do_object_collision(std::pair<double, double> collision_vels);

    void move(double dt);
};
