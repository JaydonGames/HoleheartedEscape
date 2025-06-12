#pragma once
#include "graphics/types.hpp"
#include "square.hpp"

class PhysicsWorld {
    public:
        PhysicsWorld();

        void add_square(Square* square);

        Square* get_square(int i);

        void update(double dt);

    private:
        std::vector<Square*> m_squares;
        Math::Vec2<float> m_gravity;

        void update_positions(double dt);

        void apply_gravity();

        void satisfy_constraints();

        void solve_collisions();

        void resolve_collision(auto& object_1, auto& object_2, Math::Vec2<float> normal, double depth);
};
