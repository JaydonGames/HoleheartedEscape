#include "game/player.hpp"
#include <SDL_events.h>
#include "graphics/types.hpp"
#include <cstdlib>
#include <iostream>
#include <utility>

Player::Player(Tiled::Layer collision_layer, ObjectTest &collision_object) : collision_object(&collision_object) {
    m_vel_x = 0;
    m_vel_y = 0;
    m_acceleration = 0;

    m_rect.x = 80;
    m_rect.y = 224;
    m_rect.w = 16;
    m_rect.h = 16;

    m_is_jumping = false;

    this->collision_layer = collision_layer;
}

void Player::input(SDL_Event &e) {
    if (e.type == SDL_KEYDOWN) {
        switch (e.key.keysym.sym) {
            case SDLK_UP:
                if (!m_is_jumping) {
                    m_vel_y = -PLAYER_JUMP_FORCE;
                    m_is_jumping = true;
                }
                break;
            case SDLK_RIGHT:
                m_acceleration = PLAYER_ACCELERATION;
                break;
            case SDLK_LEFT:
                m_acceleration = -PLAYER_ACCELERATION;
                break;
        }
    }
    if (e.type == SDL_KEYUP) {
        switch (e.key.keysym.sym) {
            case SDLK_RIGHT:
                m_acceleration = 0;
                break;
            case SDLK_LEFT:
                m_acceleration = 0;
                break;
        }
    }
}

void Player::move(double dt) {
    if (m_acceleration == 0) {
        if (!m_is_jumping) {
            m_friction_force = .5 * PLAYER_GRAVITY;
            if (abs(m_vel_x) > m_friction_force * dt) {
                m_vel_x -= m_vel_x * m_friction_force * dt;
            } else {
                m_vel_x = 0;
            }
        } else {
            m_drag_force = 1 * m_vel_x * abs(m_vel_x);
            m_vel_x -= m_drag_force * dt;
        }
    }
    m_vel_x += .5 * m_acceleration * dt * dt;

    if (m_vel_x > PLAYER_MAX_SPD * dt) {
        m_vel_x = PLAYER_MAX_SPD * dt;
    } else if (m_vel_x < -PLAYER_MAX_SPD * dt) {
        m_vel_x = -PLAYER_MAX_SPD * dt;
    }
    m_rect.x += m_vel_x;
    std::cout << m_vel_x << "\n";

    do_layer_collision("horizontal");
    do_object_collision("horizontal");

    m_vel_y += PLAYER_GRAVITY * dt;
    if (m_vel_y > 30) {
        m_vel_y = 30;
    }
    m_rect.y += m_vel_y;
    do_layer_collision("vertical");
    do_object_collision("vertical");
    if (m_rect.y > (1000 - m_rect.h) / 4) {
        m_rect.y = (1000 - m_rect.h) / 4;
        m_vel_y = 0;
        m_is_jumping = false;
    }
}

void Player::do_layer_collision(std::string direction) {
    for (int y = 0; y < collision_layer.tiles.size(); ++y) {
        for (int x = 0; x < collision_layer.tiles[y].size(); ++x) {
            Tiled::Tile &tile = collision_layer.tiles[y][x];
            if (tile.empty)
                continue;
            bool x_overlap = (x * 16 < m_rect.x + 16) && (x * 16 + 16 > m_rect.x);
            bool y_overlap = (y * 16 < m_rect.y + 16) && (y * 16 + 16 > m_rect.y);
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
                        m_rect.y = y * 16 - 16;
                        m_vel_y = 0;
                        m_is_jumping = false;
                    }
                }
            }
        }
    }
}

void Player::do_object_collision(std::string direction) {
    Render::Rect collision_rect = collision_object->get_rect();
    int x = collision_rect.x;
    int y = collision_rect.y;
    bool x_overlap = (x < m_rect.x + 16) && (x + 16 > m_rect.x);
    bool y_overlap = (y < m_rect.y + 16) && (y + 16 > m_rect.y);
    if (x_overlap && y_overlap) {
        if (direction == "horizontal") {
            if (m_vel_x > 0) {
                m_rect.x = x - 16;
                collision_object->m_vel_x += 10;
                collision_object->m_acceleration = 10;
            } else if (m_vel_x < 0) {
                m_rect.x = x + 16;
                collision_object->m_vel_x += 10;
                collision_object->m_acceleration = -10;
            }
        } else {
            if (m_vel_y < 0) {
                m_rect.y = y + 16;
                m_vel_y = 0;
            } else if (m_vel_y > 0) {
                m_rect.y = y - 16;
                m_vel_y = 0;
                m_is_jumping = false;
            }
        }
    }
}

Render::Rect Player::get_rect() {
    return m_rect;
}

void Player::update(double dt) {
    move(dt);
}
