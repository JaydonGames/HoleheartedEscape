#pragma once
#include "graphics/types.hpp"

struct VerletParticle {
    Math::Vec2<float> curr_position;
    Math::Vec2<float> prev_position;
    Math::Vec2<float> acceleration;
    Math::Vec2<float> force;
    float mass;
    bool is_static;

    VerletParticle(Math::Vec2<float> curr_pos, float mass, bool is_static = false);

    void update_position(double dt);

    void apply_force(Math::Vec2<float> f);
};

struct Constraint {
    Constraint(int a, int b, float l);

    int particle_a, particle_b;
    float rest_length;
};

struct Projection {
    Projection(float min, float max);
    float min, max;

    bool is_overlap(Projection& other);

    double get_overlap(Projection& other);

    bool do_flip_direction(Projection& other);
};
