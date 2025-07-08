#define _USE_MATH_DEFINES
#include "graphics/types.hpp"
#include "physics/square.hpp"
#include <algorithm>
#include <array>
#include <cmath>
#include <iostream>

VerletParticle::VerletParticle(Math::Vec2<float> pos, float mass, bool is_static)
    : curr_position(pos),
      prev_position(pos),
      mass(mass),
      is_static(is_static) {};

void VerletParticle::update_position(double dt) {
    const Math::Vec2<float> displacment = curr_position - prev_position;
    prev_position = curr_position;

    acceleration = force / mass;
    curr_position = curr_position + displacment + acceleration * dt * dt;

    force = Math::Vec2<float>();
    acceleration = Math::Vec2<float>();
}

void VerletParticle::apply_force(Math::Vec2<float> f) {
    force = force + f;
}

Constraint::Constraint(int a, int b, float l)
    : particle_a(a),
      particle_b(b),
      rest_length(l) {};

Projection::Projection(float min, float max)
    : min(min),
      max(max) {};

bool Projection::is_overlap(Projection& other) {
    return max >= other.min && other.max >= min;
}

double Projection::get_overlap(Projection& other) {
    return std::min(max - other.min, other.max - min);
}

bool Projection::do_flip_direction(Projection& other) {
    return max - other.min < other.max - min;
}
