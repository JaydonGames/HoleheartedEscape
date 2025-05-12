#pragma once
#include <SDL_events.h>
#include <utility>
#include "graphics/types.hpp"
#include "tiled/map.hpp"
#include "game/object.hpp"

class Player {
public:
    // NOTE: Use constexrp here??
    // I have no idea what units thse are in lol
    static constexpr int PLAYER_MAX_SPD = 300;
    static constexpr int PLAYER_JUMP_FORCE = 15;
    static constexpr int PLAYER_GRAVITY = 100;
    static constexpr int PLAYER_MASS = 5;
    static constexpr int PLAYER_FORCE = 21000;
    static constexpr int PLAYER_ACCELERATION = PLAYER_FORCE / PLAYER_MASS;

    Player(Tiled::Layer collision_layer, ObjectTest &collision_object);

    void input(SDL_Event &e);

    Render::Rect get_rect();

    void update(double dt);

private:
    Render::Rect m_rect;

    double m_vel_x, m_vel_y, m_acceleration, m_friction_force, m_drag_force;

    bool m_is_jumping;

    Tiled::Layer collision_layer;

    ObjectTest *collision_object;

    void move(double dt);

    void do_layer_collision(std::string direction);

    void do_object_collision(std::string direction);
};
