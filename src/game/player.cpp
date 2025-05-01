#include "game/player.hpp"
#include <SDL_events.h>
#include "graphics/types.hpp"

Player::Player(Tiled::Layer collision_layer) {
    m_vel_x = 0;
    m_vel_y = 0;

    m_rect.x = 80;
    m_rect.y = 224;
    m_rect.w = 16;
    m_rect.h = 16;

    m_gravity = 100;

    m_is_jumping = false;

    this->collision_layer = collision_layer;
}

void Player::input(SDL_Event &e) {
    if (e.type == SDL_KEYDOWN && e.key.repeat == 0) {
        switch (e.key.keysym.sym) {
            case SDLK_UP:
                if (!m_is_jumping) {
                    m_vel_y = -12;
                    m_is_jumping = true;
                }
                break;
            case SDLK_LEFT:
                m_vel_x -= PLAYER_VEL;
                break;
            case SDLK_RIGHT:
                m_vel_x += PLAYER_VEL;
                break;
        }
    } else if (e.type == SDL_KEYUP && e.key.repeat == 0) {
        switch (e.key.keysym.sym) {
            case SDLK_LEFT:
                m_vel_x += PLAYER_VEL;
                break;
            case SDLK_RIGHT:
                m_vel_x -= PLAYER_VEL;
                break;
        }
    }
}

void Player::move(double dt) {
    m_rect.x += m_vel_x * dt;
    collision("horizontal");

    m_vel_y += m_gravity * dt;
    if (m_vel_y > 30) {
        m_vel_y = 30;
    }
    m_rect.y += m_vel_y;
    collision("vertical");
    if (m_rect.y > (1000 - m_rect.h) / 4) {
        m_rect.y = (1000 - m_rect.h) / 4;
        m_vel_y = 0;
        m_is_jumping = false;
    }
}

void Player::collision(std::string direction) {
    for (int y = 0; y < collision_layer.tiles.size(); ++y) {
        for (int x = 0; x < collision_layer.tiles[y].size(); ++x) {
            Tiled::Tile &tile = collision_layer.tiles[y][x];
            if (tile.empty)
                continue;
            bool x_overlap = (x * 16 < m_rect.x + 16) && (x * 16 + 16 > m_rect.x);
            bool y_overlap = (y * 16 < m_rect.y + 16) && ((y * 16) + 16 > m_rect.y);
            if (x_overlap && y_overlap) {
                if (direction == "horizontal") {
                    if (m_vel_x > 0) {
                        m_rect.x = x * 16 - 16;
                    } else if (m_vel_x < 0) {
                        m_rect.x = x * 16 + 16;
                    }
                } else {
                    if (m_vel_y < 0) {
                        m_rect.y = y * 16 + 16;
                        m_vel_y = 0;
                    } else if (m_vel_y > 0) {
                        m_rect.y = (y * 16) - 16;
                        m_vel_y = 0;
                        m_is_jumping = false;
                    }
                }
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
