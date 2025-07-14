#include "game/player.hpp"
#include <SDL_events.h>
#include <SDL_keyboard.h>
#include "graphics/types.hpp"
#include "structures.hpp"
#include <algorithm>
#include <cstdlib>
#include <limits>

Math::Vec2<float> Player::PLAYER_JUMP_FORCE;

Player::Player(Math::Vec2<float> pos, float mass, float side_length)
    : Square(pos, mass, side_length, false, true) {
    PLAYER_JUMP_FORCE = Math::Vec2<float>(0, -30000.0f);
    m_is_jumping = false;
};

void Player::input() {
    const Uint8* keystate = SDL_GetKeyboardState(NULL);

    if (!m_is_jumping) {
        if (keystate[SDL_SCANCODE_UP]) {
            reset_y_force();
            apply_force(PLAYER_JUMP_FORCE);
            m_is_jumping = true;
        }
    }
    if (keystate[SDL_SCANCODE_LEFT]) {
        apply_force(Math::Vec2<float>(-PLAYER_SPD, 0));
    }

    if (keystate[SDL_SCANCODE_RIGHT]) {
        apply_force(Math::Vec2<float>(PLAYER_SPD, 0));
    }
}

void Player::reset_y_force() {
    for (VerletParticle particle : get_particles()) {
        particle.force.y = 0.0f;
    }
}

void Player::check_to_enable_player_jump(ArrayRef<VerletParticle> collided_object_particles) {
    // NOTE: A higher 'height' = lower y value, more negative
    float other_highest_y = std::numeric_limits<float>::infinity();
    for (VerletParticle particle : collided_object_particles) {
        other_highest_y = std::min(other_highest_y, particle.curr_position.y);
    }

    if (other_highest_y > get_center_position().y) {
        m_is_jumping = false;
    }
}
