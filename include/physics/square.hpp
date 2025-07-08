#pragma once
#include "graphics/types.hpp"
#include "physics/structures.hpp"
#include "shape.hpp"
#include <array>

class Square : public Shape {
public:
    float side_length;
    float diagonal_length;
    float mass;
    float angle;

    Square(Math::Vec2<float> pos, float mass, float side_length = 16.0f, bool is_static = false);

    Square(Square&& other) noexcept;

    Square& operator=(Square&& other) noexcept;

    Math::Vec2<float> get_curr_position() override;

    Math::Vec2<float> get_center_position() override;

    float get_max_side_length() override;

    std::array<VerletParticle*, 4> get_particles() override;

    std::array<Constraint*, 6> get_constraints() override;

    std::array<Math::Vec2<float>, 2> get_axes() override;

    Projection project(Math::Vec2<float> axis) override;

    void apply_force(Math::Vec2<float> f) override;

    float get_angle() override;

private:
    std::array<VerletParticle, 4> m_vertices;
    std::array<Constraint, 6> m_constraints;
};
