#pragma once
#include <SDL_events.h>
#include "graphics/types.hpp"
#include "tiled/map.hpp"

class Player {
public:
    // NOTE: Use constexrp here??
    static constexpr int PLAYER_VEL = 300;

    Player(Tiled::Layer collision_layer);

    void input(SDL_Event &e);

    Render::Rect get_rect();

    void update(double dt);

private:
    Render::Rect m_rect;

    double m_vel_x, m_vel_y, m_gravity;

    bool m_is_jumping;

    Tiled::Layer collision_layer;

    void move(double dt);

    void collision(std::string direction);
};
