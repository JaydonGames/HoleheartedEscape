#include "graphics/types.hpp"
#include "physics/square.hpp"
#include <algorithm>
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

Projection::Projection(float min, float max) : min(min), max(max) {};

bool Projection::is_overlap(Projection& other) {
    return max >= other.min && other.max >= min;
}

double Projection::get_overlap(Projection& other) {
    return std::min(max - other.min, other.max - min);
}

bool Projection::do_flip_direction(Projection& other) {
    return max - other.min < other.max - min;
}

Square::Square(Render::Vector2D pos, float side_length, bool is_static)
    : is_static(is_static),
      side_length(side_length),
      m_vertices{
          VerletParticle(pos, is_static),
          VerletParticle(pos + Render::Vector2D(side_length, 0), is_static),
          VerletParticle(pos + Render::Vector2D(side_length, side_length), is_static),
          VerletParticle(pos + Render::Vector2D(0, side_length), is_static),
      } {
    m_constraints[0] = std::make_unique<Constraint>(0, 1, side_length);
    m_constraints[1] = std::make_unique<Constraint>(1, 2, side_length);
    m_constraints[2] = std::make_unique<Constraint>(2, 3, side_length);
    m_constraints[3] = std::make_unique<Constraint>(3, 0, side_length);
}

Square::Square(Square&& other) noexcept : m_vertices(std::move(other.m_vertices)) {
    m_constraints = std::move(other.m_constraints);
    side_length = other.side_length;
    is_static = other.is_static;
}

Square& Square::operator=(Square&& other) noexcept {
    if (this != &other) {
        m_vertices = std::move(other.m_vertices);
        m_constraints = std::move(other.m_constraints);
        side_length = other.side_length;
        is_static = other.is_static;
    }
    return *this;
}

Render::Vector2D Square::get_curr_position() {
    return m_vertices[0].curr_position;
}

Render::Vector2D Square::get_center_position() {
    return m_vertices[0].curr_position + Render::Vector2D(side_length / 2, side_length / 2);
}

std::array<VerletParticle*, 4> Square::get_particles() {
    std::array<VerletParticle*, 4> ptrs;
    for (int i = 0; i < 4; ++i) {
        ptrs[i] = &m_vertices[i];
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

std::array<Render::Vector2D, 2> Square::get_axes() {
    std::array<Render::Vector2D, 2> axes;
    for (int i = 0; i < 2; ++i) {
        VerletParticle p1 = m_vertices[i];
        VerletParticle p2 = m_vertices[i + 1];

        Render::Vector2D edge = p1.curr_position - p2.curr_position;
        Render::Vector2D normal = edge.get_perpendicular().normalize();

        axes[i] = normal;
    }
    return axes;
}

Projection Square::project(Render::Vector2D axis) {
    double min = m_vertices[0].curr_position.dot_product(axis);
    double max = min;
    for (VerletParticle& particle : m_vertices) {
        double p = particle.curr_position.dot_product(axis);
        if (p < min) {
            min = p;
        } else if (p > max) {
            max = p;
        }
    }
    Projection proj = Projection(min, max);
    return proj;
}

void Square::accelerate(Render::Vector2D a) {
    for (VerletParticle& p : m_vertices) {
        p.accelerate(a);
    }
}
