#include "game/player.hpp"
#include <SDL_events.h>
#include <SDL_keyboard.h>
#include "graphics/types.hpp"
#include <cstdlib>

Math::Vec2<float> Player::PLAYER_JUMP_FORCE;

Player::Player(Math::Vec2<float> pos, float mass, float side_length)
    : Square(pos, mass, side_length) {
    PLAYER_JUMP_FORCE = Math::Vec2<float>(0, -15000.0f);
    is_jumping = false;
};

void Player::input() {
    const Uint8* keystate = SDL_GetKeyboardState(NULL);

    if (!is_jumping) {
        if (keystate[SDL_SCANCODE_UP]) {
            apply_force(PLAYER_JUMP_FORCE);
            // is_jumping = true;
        }
    }
    if (keystate[SDL_SCANCODE_LEFT]) {
        apply_force(Math::Vec2<float>(-PLAYER_SPD, 0));
    }

    if (keystate[SDL_SCANCODE_RIGHT]) {
        apply_force(Math::Vec2<float>(PLAYER_SPD, 0));
    }
}
