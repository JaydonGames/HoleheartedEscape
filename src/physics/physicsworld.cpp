#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <vector>
#include "graphics/types.hpp"
#include "physics/square.hpp"
#include "physics/physicsworld.hpp"
#include <iostream>

PhysicsWorld::PhysicsWorld() {
    m_gravity = Render::Vector2D(0, 10.0f);
};

void PhysicsWorld::add_square(Square& square) {
    m_squares.push_back(&square);
}

void PhysicsWorld::update(double dt) {
    apply_gravity(dt);
    update_positions(dt);
    satisfy_constraints();
}

void PhysicsWorld::apply_gravity(double dt) {
    for (Square* square : m_squares) {
        std::array<VerletParticle*, 4> particles = square->get_particles();
        for (VerletParticle* particle : particles) {
            particle->accelerate(m_gravity);
        }
    }
}

void PhysicsWorld::update_positions(double dt) {
    for (Square* square : m_squares) {
        std::array<VerletParticle*, 4> particles = square->get_particles();
        for (VerletParticle* particle : particles) {
            particle->update_position(dt);
        }
    }
}

void PhysicsWorld::satisfy_constraints() {
    for (Square* square : m_squares) {
        std::array<Constraint*, 4> constraints = square->get_constraints();
        std::array<VerletParticle*, 4> particles = square->get_particles();
        for (int j = 0; j < 5; ++j) {
            for (int i = 0; i < 4; ++i) {
                Constraint* c = constraints[i];
                // std::cout << c->particle_a << '\n';
                Render::Vector2D particle_1 = particles[c->particle_a]->curr_position;
                Render::Vector2D particle_2 = particles[c->particle_b]->curr_position;

                Render::Vector2D delta = particle_2 - particle_1;
                float delta_length = std::sqrt(delta.dot_product(delta));
                float diff = (delta_length - c->rest_length) / delta_length;
                particle_1 = particle_1 - delta * .5 * diff;
                particle_2 = particle_2 + delta * .5 * diff;
            }
        }
    }
}

// void PhysicsWorld::solve_collisions() {
//     for (int i = 0; i < m_squares.size(); ++i) {
//         Square* square_1 = m_squares[i];
//         std::cout << square_1->.y << '\n';
//         for (int j = i + 1; j < m_squares.size(); ++j) {
//             Square* square_2 = m_squares[j];
//             float x_overlap = 16 - abs(square_1->curr_position.x - square_2->curr_position.x);
//             float y_overlap = 16 - abs(square_1->curr_position.y - square_2->curr_position.y);
//             if (x_overlap > 0 && y_overlap > 0) {
//                 R
//             }
//         }
//     }
// }
