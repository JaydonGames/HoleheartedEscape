#pragma once
#include <SDL_events.h>
#include "graphics/types.hpp"

class Player {
public:
    // NOTE: Use constexrp here??
    static const int PLAYER_VEL = 300;

    Player();

    void input(SDL_Event &e);

    void move(double dt);

    Render::Rect get_rect();

private:
    Render::Rect m_rect;

    double m_vel_x, m_vel_y, m_gravity;

    bool is_jumping;
};
