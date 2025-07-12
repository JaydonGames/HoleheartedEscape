#pragma once
#include "graphics/types.hpp"
#include "physics/structures.hpp"
#include <vector>

struct Shape {
    bool is_static;
    float mass;
    float angle;

    Shape(bool is_static, float mass);

    virtual Math::Vec2<float> get_curr_position() = 0;

    virtual Math::Vec2<float> get_center_position() = 0;

    virtual float get_max_side_length() = 0;

    virtual ArrayRef<VerletParticle> get_particles() = 0;

    virtual ArrayRef<Constraint> get_constraints() = 0;

    virtual std::vector<Math::Vec2<float>> get_axes() = 0;

    virtual Projection project(Math::Vec2<float> axis) = 0;

    virtual void apply_force(Math::Vec2<float> f) = 0;

    virtual float get_angle() = 0;
};
