#pragma once
#include "graphics/types.hpp"
#include "physics/structures.hpp"
#include "shape.hpp"
#include <array>
#include <vector>

class Square : public Shape {
public:
    float side_length;
    float diagonal_length;
    float angle;
    bool is_ground;

    Square(Math::Vec2<float> pos, float mass, float side_length = 16.0f, bool is_static = false,
           bool is_player = false);

    Square(Square&& other) noexcept;

    Square& operator=(Square&& other) noexcept;

    Math::Vec2<float> get_curr_position() override;

    Math::Vec2<float> get_center_position() override;

    float get_max_side_length() override;

    ArrayRef<VerletParticle> get_particles() override;

    ArrayRef<Constraint> get_constraints() override;

    std::vector<Math::Vec2<float>> get_axes() override;

    Projection project(Math::Vec2<float> axis) override;

    void apply_force(Math::Vec2<float> f) override;

    float get_angle() override;

    // WARNING: This is terrible but I don't feel like rewriting everything lol
    // It's only for the player class
    // I also don't know how else to do this. Aydon help
    void check_to_enable_player_jump(ArrayRef<VerletParticle>) override;

private:
    std::array<VerletParticle, 4> m_vertices;
    std::array<Constraint, 6> m_constraints;
};
