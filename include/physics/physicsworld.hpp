#pragma once
#include <array>
#include <vector>
#include "graphics/types.hpp"
#include "spatial_hashing_grid.hpp"
#include "square.hpp"

class PhysicsWorld {
    public:
        PhysicsWorld(SpatialHashingGrid);

        void add_object(Square* square);

        Square* get_square(int i);

        void update(double dt);

    private:
        SpatialHashingGrid spatial_grid;
        std::vector<Square*> m_all_objects;
        std::vector<Square*> m_nonstatic_objects;
        std::vector<Square*> m_static_objects;
        Math::Vec2<float> m_gravity;
        const int sub_steps = 8;

        void update_positions(Square* square, double dt);

        void apply_gravity(Square* square);

        void satisfy_constraints(Square* square);

        void solve_collisions();

        void resolve_collision(auto& object_1, auto& object_2, std::vector<VerletParticle*> colliding_particles_1, std::vector<VerletParticle*> colliding_particles_2, Math::Vec2<float> normal, double depth);

        std::vector<VerletParticle*> get_collision_particles(std::array<VerletParticle*, 4>, Math::Vec2<float> normal);
};
