#pragma once
#include "graphics/types.hpp"
#include "square.hpp"

class PhysicsWorld {
    public:
        PhysicsWorld();

        void add_square(Square &square);

        void update(double dt);

    private:
        std::vector<Square *> m_squares;
        Render::Vector2D m_gravity;

        void update_positions(double dt);

        void apply_gravity(double dt);

        void satisfy_constraints();

        void solve_collisions();

        void resolve_collision();
};
