#pragma once
#include <SDL_events.h>
#include <utility>
#include "graphics/types.hpp"
#include "tiled/map.hpp"
#include "game/object.hpp"
#include "physics/square.hpp"

// class Player : public Square {
// public:
//     static constexpr float PLAYER_MAX_SPD = 300;
//     static constexpr float PLAYER_JUMP_FORCE = 40;
//
//     Player(Render::Vector2D pos);
//
//     void input(SDL_Event &e);
//
// private:
//     bool m_is_jumping;
// };
