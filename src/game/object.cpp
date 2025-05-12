#include <utility>
#include "game/object.hpp"
#include <iostream>

ObjectTest::ObjectTest(int x, int y, Tiled::Layer collision_layer) {
    m_vel_x = 0;
    m_vel_y = 0;
    m_gravity = 100;
    m_acceleration = 10;

    m_rect.x = x;
    m_rect.y = y;
    m_rect.w = 16;
    m_rect.h = 16;

    this->collision_layer = collision_layer;
}

void ObjectTest::do_layer_collision(std::string direction) {
    for (int y = 0; y < collision_layer.tiles.size(); ++y) {
        for (int x = 0; x < collision_layer.tiles[y].size(); ++x) {
            Tiled::Tile &tile = collision_layer.tiles[y][x];
            if (tile.empty || y * 16 > m_rect.y)
                continue;
            bool x_overlap = (x * 16 < m_rect.x + 16) && (x * 16 + 16 > m_rect.x);
            bool y_overlap = (y * 16 < m_rect.y + 16) && ((y * 16) + 16 > m_rect.y);
            if (x_overlap && y_overlap) {
                if (direction == "horizontal") {
                    if (m_vel_x > 0) {
                        m_rect.x = x * 16 - 16;
                        m_vel_x = 0;
                    } else if (m_vel_x < 0) {
                        m_rect.x = x * 16 + 16;
                        m_vel_x = 0;
                    }
                } else {
                    if (m_vel_y < 0) {
                        m_rect.y = y * 16 + 16;
                        m_vel_y = 0;
                    } else if (m_vel_y > 0) {
                        m_rect.y = (y * 16) - 16;
                        m_vel_y = 0;
                    }
                }
            }
        }
    }
}

void ObjectTest::do_object_collision(std::pair<double, double> collision_vels) {}

void ObjectTest::move(double dt) {
    m_vel_x *= m_acceleration * dt;
    // std::cout << m_vel_x << '\n';
    m_rect.x += m_vel_x;
    do_layer_collision("horizontal");
}

Render::Rect ObjectTest::get_rect() {
    return m_rect;
}

void ObjectTest::update(double dt) {
    move(dt);
}
