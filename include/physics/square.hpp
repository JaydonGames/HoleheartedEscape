#pragma once
#include <array>
#include <memory>
#include <vector>
#include "graphics/types.hpp"

struct VerletParticle {
    Render::Vector2D curr_position;
    Render::Vector2D prev_position;
    Render::Vector2D acceleration;
    bool is_static;

    VerletParticle(Render::Vector2D curr_pos, bool is_static = false);

    void update_position(double dt);

    void accelerate(Render::Vector2D a);
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

class Square {
public:
    // TODO: ADD mass
    bool is_static;
    float side_length;
    float diagonal_length;

    Square(Render::Vector2D pos, float side_length = 16.0f, bool is_static = false);

    Square(Square&& other) noexcept;

    Square& operator=(Square&& other) noexcept;

    Render::Vector2D get_curr_position();

    Render::Vector2D get_center_position();

    std::array<VerletParticle*, 4> get_particles();

    std::array<Constraint*, 6> get_constraints();

    std::array<Render::Vector2D, 2> get_axes();

    Projection project(Render::Vector2D axis);

    void accelerate(Render::Vector2D a);

private:
    std::array<VerletParticle, 4> m_vertices;
    std::array<Constraint, 6> m_constraints;
};
