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
        static Render::Vector2D PLAYER_JUMP_FORCE;
        static constexpr int PLAYER_SPD = 500;

        Player(Render::Vector2D pos);

        void input();

        bool is_jumping;
};
