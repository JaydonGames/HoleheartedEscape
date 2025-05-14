#include "graphics/types.hpp"
#include "physics/square.hpp"
#include <array>
#include <iostream>
#include <memory>

VerletParticle::VerletParticle(Render::Vector2D pos, bool is_static)
    : curr_position(pos), prev_position(pos), is_static(is_static) {};

void VerletParticle::update_position(double dt) {
    const Render::Vector2D displacment = curr_position - prev_position;
    prev_position = curr_position;
    curr_position = curr_position + displacment + acceleration * dt * dt;
    acceleration = Render::Vector2D();
}

void VerletParticle::accelerate(Render::Vector2D a) {
    acceleration = acceleration + a;
}

Constraint::Constraint(int a, int b, float l) : particle_a(a), particle_b(b), rest_length(l) {};

Square::Square(Render::Vector2D pos, float side_length, bool is_static) {
    m_particles[0] = std::make_unique<VerletParticle>(pos, is_static);
    m_particles[1] = std::make_unique<VerletParticle>(pos + Render::Vector2D(side_length, 0), is_static);
    m_particles[2] = std::make_unique<VerletParticle>(pos + Render::Vector2D(0, side_length), is_static);
    m_particles[3] = std::make_unique<VerletParticle>(pos + Render::Vector2D(side_length, side_length), is_static);

    m_constraints[0] = std::make_unique<Constraint>(0, 1, side_length);
    m_constraints[1] = std::make_unique<Constraint>(1, 2, side_length);
    m_constraints[2] = std::make_unique<Constraint>(2, 3, side_length);
    m_constraints[3] = std::make_unique<Constraint>(3, 0, side_length);
}

Render::Vector2D Square::get_curr_position() {
    return m_particles[0]->curr_position;
}

std::array<VerletParticle*, 4> Square::get_particles() {
    std::array<VerletParticle*, 4> ptrs;
    for (int i = 0; i < 4; ++i) {
        ptrs[i] = m_particles[i].get();
    }
    return ptrs;
}

std::array<Constraint*, 4> Square::get_constraints() {
    std::array<Constraint*, 4> ptrs;
    for (int i = 0; i < 4; ++i) {
        ptrs[i] = m_constraints[i].get();
    }
    return ptrs;
}
