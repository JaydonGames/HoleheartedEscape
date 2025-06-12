#include "game/player.hpp"
#include <SDL_events.h>
#include <SDL_keyboard.h>
#include "graphics/types.hpp"
#include <cstdlib>
#include <iostream>
#include <utility>

Math::Vec2<float> Player::PLAYER_JUMP_FORCE;

Player::Player(Math::Vec2<float> pos)
    : Square(pos) {
    PLAYER_JUMP_FORCE = Math::Vec2<float>(0, -15000.0f);
    is_jumping = false;
};

void Player::input() {
    const Uint8* keystate = SDL_GetKeyboardState(NULL);

    if (keystate[SDL_SCANCODE_UP]) {
        accelerate(PLAYER_JUMP_FORCE);
    }

    if (keystate[SDL_SCANCODE_LEFT]) {
        accelerate(Math::Vec2<float>(-PLAYER_SPD, 0));
    }

    if (keystate[SDL_SCANCODE_RIGHT]) {
        accelerate(Math::Vec2<float>(PLAYER_SPD, 0));
    }
}
