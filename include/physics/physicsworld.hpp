#pragma once
#include "graphics/types.hpp"
#include "square.hpp"

class PhysicsWorld {
    public:
        PhysicsWorld();

        int add_square(Square&& square);

        Square& get_square(int i);

        void update(double dt);

    private:
        std::vector<Square> m_squares;
        Render::Vector2D m_gravity;

        void update_positions(double dt);

        void apply_gravity(double dt);

        void satisfy_constraints();

        void solve_collisions();

        void resolve_collision(Square& square_1, Square& square_2, Render::Vector2D normal, double depth);
};
