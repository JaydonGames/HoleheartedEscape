#pragma once
#include "graphics/types.hpp"
#include "physics/structures.hpp"
#include <vector>

struct Shape {
    bool is_static;
    float mass;
    float angle;
    bool is_player;

    Shape(bool is_static, float mass, bool is_player);

    virtual Math::Vec2<float> get_curr_position() = 0;

    virtual Math::Vec2<float> get_center_position() = 0;

    virtual float get_max_side_length() = 0;

    virtual ArrayRef<VerletParticle> get_particles() = 0;

    virtual ArrayRef<Constraint> get_constraints() = 0;

    virtual std::vector<Math::Vec2<float>> get_axes() = 0;

    virtual Projection project(Math::Vec2<float> axis) = 0;

    virtual void apply_force(Math::Vec2<float> f) = 0;

    virtual float get_angle() = 0;

    // WARNING: This is terrible but I don't feel like rewriting everything lol
    // It's only for the player class
    // I also don't know how else to do this. Aydon help
    virtual void check_to_enable_player_jump(ArrayRef<VerletParticle> collided_object_particles) = 0;
};
