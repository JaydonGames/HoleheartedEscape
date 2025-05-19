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
    m_gravity = Render::Vector2D(0, 50.0f);
};

int PhysicsWorld::add_square(Square&& square) {
    m_squares.push_back(std::move(square));
    return m_squares.size() - 1;
}

Square& PhysicsWorld::get_square(int i) {
    return m_squares[i];
}

// FIX: ADD the for loops here and remove them everywhere else
void PhysicsWorld::update(double dt) {
    apply_gravity(dt);
    update_positions(dt);
    satisfy_constraints();
    solve_collisions();
}

void PhysicsWorld::apply_gravity(double dt) {
    for (Square& square : m_squares) {
        if (!square.is_static) {
            std::array<VerletParticle*, 4> particles = square.get_particles();
            for (VerletParticle* particle : particles) {
                particle->accelerate(m_gravity);
            }
        }
    }
}

void PhysicsWorld::update_positions(double dt) {
    for (Square& square : m_squares) {
        if (!square.is_static) {
            std::array<VerletParticle*, 4> particles = square.get_particles();
            for (VerletParticle* particle : particles) {
                particle->update_position(dt);
            }
        }
    }
}

void PhysicsWorld::satisfy_constraints() {
    for (Square& square : m_squares) {
        if (square.is_static)
            continue;

        std::array<Constraint*, 4> constraints = square.get_constraints();
        std::array<VerletParticle*, 4> particles = square.get_particles();

        for (int j = 0; j < 10; ++j) {
            for (int i = 0; i < 4; ++i) {
                Constraint* c = constraints[i];
                VerletParticle* p1 = particles[c->particle_a];
                VerletParticle* p2 = particles[c->particle_b];

                if (p1->is_static && p2->is_static)
                    continue;

                Render::Vector2D delta = p2->curr_position - p1->curr_position;
                float delta_length = std::sqrt(delta.dot_product(delta));
                float diff = (delta_length - c->rest_length) / delta_length;

                if (p1->is_static) {
                    p2->curr_position = p2->curr_position - delta * diff;
                } else if (p2->is_static) {
                    p1->curr_position = p1->curr_position + delta * diff;
                } else {
                    p1->curr_position = p1->curr_position + delta * diff * 0.5f;
                    p2->curr_position = p2->curr_position - delta * diff * 0.5f;
                }
            }
        }
    }
}

void PhysicsWorld::solve_collisions() {
    for (int j = 0; j < m_squares.size(); ++j) {
        Square& square_1 = m_squares[j];
        std::array<Render::Vector2D, 2> axes_1 = square_1.get_axes();
        for (int i = (j + 1); i < m_squares.size(); ++i) {
            bool is_collide = true;
            Square& square_2 = m_squares[i];
            std::array<Render::Vector2D, 2> axes_2 = square_2.get_axes();

            /*
             NOTE: The normal or depth does not indicate which direction
             to move the objects in. It only indicates on what axis to move(normal) and how much(depth).
             The bool will flip the direction by making overlap negative
             depending on where the projections overlap
            */
            double depth = 9999999;
            Render::Vector2D normal;
            bool do_flip_direction;

            for (Render::Vector2D axis : axes_1) {
                Projection p1 = square_1.project(axis);
                Projection p2 = square_2.project(axis);

                if (!p1.is_overlap(p2)) {
                    is_collide = false;
                    break;
                } else {
                    // NOTE: get_overlap assumes there is an overlap
                    double o = p1.get_overlap(p2);
                    if (o < depth) {
                        depth = o;
                        normal = axis;
                        do_flip_direction = p1.do_flip_direction(p2);
                    }
                }
            }
            if (!is_collide)
                continue;

            for (Render::Vector2D axis : axes_2) {
                Projection p1 = square_1.project(axis);
                Projection p2 = square_2.project(axis);

                if (!p1.is_overlap(p2)) {
                    is_collide = false;
                    break;
                } else {
                    // NOTE: get_overlap assumes there is an overlap
                    double o = p1.get_overlap(p2);
                    if (o < depth) {
                        depth = o;
                        normal = axis;
                        do_flip_direction = p1.do_flip_direction(p2);
                    }
                }
            }

            if (!is_collide)
                continue;

            if (do_flip_direction) {
                depth = depth * -1.0f;
            }

            resolve_collision(square_1, square_2, normal, depth);
        }
    }
}

void PhysicsWorld::resolve_collision(Square& square_1, Square& square_2, Render::Vector2D normal, double depth) {
    if (square_1.is_static) {
        for (VerletParticle* particle : square_2.get_particles()) {
            particle->curr_position = particle->curr_position + normal * depth;
        }
    } else if (square_2.is_static) {
        for (VerletParticle* particle : square_1.get_particles()) {
            particle->curr_position = particle->curr_position + normal * depth;
        }
    } else {
        for (VerletParticle* particle : square_1.get_particles()) {
            particle->curr_position = particle->curr_position - normal * (depth / 2.0f);
        }
        for (VerletParticle* particle : square_2.get_particles()) {
            particle->curr_position = particle->curr_position + normal * (depth / 2.0f);
        }
    }
}
