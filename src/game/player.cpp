#include "game/player.hpp"
#include <SDL_events.h>
#include "graphics/types.hpp"

Player::Player() {
    m_vel_x = 0;
    m_vel_y = 0;

    m_rect.x = 0;
    m_rect.y = 0;
    m_rect.w = 16;
    m_rect.h = 16;

    m_gravity = 100;

    is_jumping = false;
}

void Player::input(SDL_Event &e) {
    if (e.type == SDL_KEYDOWN && e.key.repeat == 0) {
        switch (e.key.keysym.sym) {
            case SDLK_UP:
                if (!is_jumping) {
                    m_vel_y = -20;
                    is_jumping = true;
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

    m_vel_y += m_gravity * dt;
    if (m_vel_y > 30) {
        m_vel_y = 30;
    }
    m_rect.y += m_vel_y;
    if (m_rect.y > (1000 - m_rect.h) / 4) {
        m_rect.y = (1000 - m_rect.h) / 4;
        m_vel_y = 0;
        is_jumping = false;
    }
}

Render::Rect Player::get_rect() {
    return m_rect;
}
