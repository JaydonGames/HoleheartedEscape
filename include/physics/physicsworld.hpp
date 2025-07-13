#pragma once
#include <vector>
#include "graphics/types.hpp"
#include "spatial_hashing_grid.hpp"
#include "shape.hpp"
#include "structures.hpp"

class PhysicsWorld {
public:
    PhysicsWorld(SpatialHashingGrid);

    void add_object(Shape* square);

    Shape* get_object(int i);

    void update(double dt);

private:
    SpatialHashingGrid spatial_grid;
    std::vector<Shape*> m_all_objects;
    std::vector<Shape*> m_nonstatic_objects;
    std::vector<Shape*> m_static_objects;
    Math::Vec2<float> m_gravity;
    const int sub_steps = 8;

    void update_positions(Shape* square, double dt);

    void apply_gravity(Shape* square);

    void satisfy_constraints(Shape* square);

    void solve_collisions();

    void resolve_collision(auto& object_1, auto& object_2, std::vector<VerletParticle*> colliding_particles_1,
                           std::vector<VerletParticle*> colliding_particles_2, Math::Vec2<float> normal, double depth);

    std::vector<VerletParticle*> get_collision_particles(ArrayRef<VerletParticle> particles, Math::Vec2<float> normal);
};
