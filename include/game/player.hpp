#pragma once
#include <SDL_events.h>
#include "graphics/types.hpp"
#include "physics/square.hpp"
#include "structures.hpp"

// TODO: Cap player speed in all directions
// add friction and drag to all objects
class Player : public Square {
public:
    static Math::Vec2<float> PLAYER_JUMP_FORCE;
    static constexpr int PLAYER_SPD = 5000;

    Player(Math::Vec2<float> pos, float mass = 70.0f, float side_length = 14.0f);

    void input();

    void check_to_enable_player_jump(ArrayRef<VerletParticle> collided_object_particles);

private:
    bool m_is_jumping;
};
