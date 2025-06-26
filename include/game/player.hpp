#pragma once
#include <SDL_events.h>
#include <utility>
#include "graphics/types.hpp"
#include "tiled/map.hpp"
#include "game/object.hpp"
#include "physics/square.hpp"

// TODO: Cap player speed in all directions
// add friction and drag to all objects
class Player : public Square {
public:
    static Math::Vec2<float> PLAYER_JUMP_FORCE;
    static constexpr int PLAYER_SPD = 2500;
    float mass;

    Player(Math::Vec2<float> pos, float mass = 70.0f);

    void input();

    bool is_jumping;
};
