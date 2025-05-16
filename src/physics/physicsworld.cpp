#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <stdexcept>
#include <vector>
#include "graphics/types.hpp"
#include "physics/square.hpp"
#include "physics/physicsworld.hpp"
#include <SDL2/SDL_audio.h>
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

void PhysicsWorld::solve_collisions() {
    for (int j = 0; j < m_squares.size(); ++j) {
        Square* square_1 = m_squares[j];
        std::array<Render::Vector2D, 2> axes_1 = square_1->get_axes();
        for (int i = (j + 1); i < m_squares.size(); ++i) {
            bool is_collide = true;
            Square* square_2 = m_squares[i];
            std::array<Render::Vector2D, 2> axes_2 = square_2->get_axes();

            double overlap = 9999999;
            Render::Vector2D min_axis;

            for (Render::Vector2D axis : axes_1) {
                Projection p1 = square_1->project(axis);
                Projection p2 = square_2->project(axis);

                // Do the projections overlap?
                if (!p1.is_overlap(p2)) {
                    // Then we can guarentee the shapes do not overlap due to SAT theorem
                    // Continue to check collision for next shape
                    is_collide = false;
                    break;
                } else {
                    double o = p1.get_overlap(p2);
                    if (o < overlap) {
                        overlap = o;
                        min_axis = axis;
                    }
                }
            }
            // Continue to check collision for next shape
            if (!is_collide)
                continue;

            for (Render::Vector2D axis : axes_2) {
                Projection p1 = square_1->project(axis);
                Projection p2 = square_2->project(axis);

                // Do the projections overlap?
                if (!p1.is_overlap(p2)) {
                    // Then we can guarentee the shapes do not overlap due to SAT theorem
                    // Continue to check collision for next shape
                    is_collide = false;
                    break;
                } else {
                    double o = p1.get_overlap(p2);
                    if (o < overlap) {
                        overlap = o;
                        min_axis = axis;
                    }
                }
            }
        }
    }
}

void PhysicsWorld::resolve_collision() {}
