#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <execution>
#include <limits>
#include <stdexcept>
#include <utility>
#include <vector>
#include "graphics/types.hpp"
#include "physics/square.hpp"
#include "spatial_hashing_grid.hpp"
#include "physics/physicsworld.hpp"
#include <iostream>

// TODO: Clean up if (square->is_static)
PhysicsWorld::PhysicsWorld(SpatialHashingGrid spatial_grid)
    : spatial_grid(spatial_grid) {
    m_gravity = Math::Vec2<float>(0, 500.0f);
};

void PhysicsWorld::add_object(Square* square) {
    m_all_objects.push_back(square);
    if (square->is_static) {
        m_static_objects.push_back(square);
    } else {
        m_nonstatic_objects.push_back(square);
    }

    spatial_grid.add_object(square);
}

Square* PhysicsWorld::get_square(int i) {
    return m_all_objects[i];
}

void PhysicsWorld::update(double dt) {
    const float sub_dt = dt / (float)sub_steps;
    for (int i = sub_steps; i > 0; i--) {
        for (Square* square : m_all_objects) {
            if (!square->is_static) {
                apply_gravity(square);
                satisfy_constraints(square);
            }
        }
        solve_collisions();
        for (Square* square : m_all_objects) {
            update_positions(square, sub_dt);
            spatial_grid.update_object(square);
        }
    }
}

void PhysicsWorld::apply_gravity(Square* square) {
    square->apply_force(m_gravity);
}

void PhysicsWorld::update_positions(Square* square, double dt) {
    if (!square->is_static) {
        std::array<VerletParticle*, 4> particles = square->get_particles();
        for (VerletParticle* particle : particles) {
            particle->update_position(dt);
        }
    }
}

void PhysicsWorld::satisfy_constraints(Square* square) {
    if (!square->is_static) {
        std::array<Constraint*, 6> constraints = square->get_constraints();
        std::array<VerletParticle*, 4> particles = square->get_particles();

        for (int j = 0; j < 5; ++j) {
            for (int i = 0; i < 6; ++i) {
                Constraint* c = constraints[i];
                VerletParticle* p1 = particles[c->particle_a];
                VerletParticle* p2 = particles[c->particle_b];

                if (p1->is_static && p2->is_static) {
                    continue;
                }

                Math::Vec2<float> delta = p2->curr_position - p1->curr_position;
                delta = delta * ((c->rest_length * c->rest_length) /
                                     ((delta.dot_product(delta)) + (c->rest_length * c->rest_length)) -
                                 0.5f);
                p2->curr_position = p2->curr_position + delta;
                p1->curr_position = p1->curr_position - delta;
            }
        }
    }
}

// NOTE: Every non-static object will be updated in the spatial hash and checked for collisions
void PhysicsWorld::solve_collisions() {
    for (Square* square_1 : m_nonstatic_objects) {
        std::array<Math::Vec2<float>, 2> axes_1 = square_1->get_axes();
        std::vector<Square*> closest_objects = spatial_grid.get_closest_objects(square_1);
        for (Square* square_2 : closest_objects) {
            std::array<Math::Vec2<float>, 2> axes_2 = square_2->get_axes();

            /*
             NOTE: The normal or depth does not indicate which direction
             to move the objects in. It only indicates on what axis to move(normal) and how much(depth).
             The bool will flip the direction by making overlap negative
             depending on where the projections overlap
            */
            double depth = std::numeric_limits<double>::infinity();
            Math::Vec2<float> normal;
            bool do_flip_direction;
            bool is_collide = true;

            for (Math::Vec2<float> axis : axes_1) {
                Projection p1 = square_1->project(axis);
                Projection p2 = square_2->project(axis);

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

            for (Math::Vec2<float> axis : axes_2) {
                Projection p1 = square_1->project(axis);
                Projection p2 = square_2->project(axis);

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

            if (is_collide and depth > .05) {
                if (do_flip_direction) {
                    depth = depth * -1.0f;
                }
                // depth = std::ceil(depth * 100.0) / 100.0;
                depth /= sub_steps;
                std::vector<VerletParticle*> colliding_particles_1 =
                    get_collision_particles(square_1->get_particles(), -normal);
                std::vector<VerletParticle*> colliding_particles_2 =
                    get_collision_particles(square_2->get_particles(), normal);
                resolve_collision(square_1, square_2, colliding_particles_1, colliding_particles_2, normal, depth);
            }
        }
    }
}

void PhysicsWorld::resolve_collision(auto& object_1, auto& object_2, std::vector<VerletParticle*> colliding_particles_1,
                                     std::vector<VerletParticle*> colliding_particles_2, Math::Vec2<float> normal,
                                     double depth) {
    if (object_1->is_static) {
        Math::Vec2<float> correction = normal * depth;
        for (VerletParticle* particle : colliding_particles_2) {
            particle->curr_position = particle->curr_position + correction;
            Math::Vec2<float> velocity = particle->curr_position - particle->prev_position;
            particle->prev_position = particle->curr_position - (velocity * 0.9f);
        }
    } else if (object_2->is_static) {
        Math::Vec2<float> correction = normal * depth;
        for (VerletParticle* particle : colliding_particles_1) {
            particle->curr_position = particle->curr_position + correction;
            Math::Vec2<float> velocity = particle->curr_position - particle->prev_position;
            particle->prev_position = particle->curr_position - (velocity * 0.9f);
        }
    } else {
        float obj_ratio_1 = object_2->mass / (object_2->mass + object_1->mass);
        float obj_ratio_2 = object_1->mass / (object_2->mass + object_1->mass);
        Math::Vec2<float> correction_1 = normal * (depth * obj_ratio_1);
        Math::Vec2<float> correction_2 = normal * (depth * obj_ratio_2);

        for (VerletParticle* particle : colliding_particles_1) {
            particle->curr_position = particle->curr_position + correction_1;
            Math::Vec2<float> velocity = particle->curr_position - particle->prev_position;
            particle->prev_position = particle->curr_position - (velocity * 0.95f);
        }
        for (VerletParticle* particle : colliding_particles_2) {
            particle->curr_position = particle->curr_position - correction_2;
            Math::Vec2<float> velocity = particle->curr_position - particle->prev_position;
            particle->prev_position = particle->curr_position - (velocity * 0.95f);
        }
    }
}

std::vector<VerletParticle*> PhysicsWorld::get_collision_particles(std::array<VerletParticle*, 4> particles,
                                                                   Math::Vec2<float> normal) {
    std::vector<VerletParticle*> colliding_particles;

    float max_projection = -std::numeric_limits<float>::infinity();
    for (VerletParticle* particle : particles) {
        float proj = particle->curr_position.dot_product(normal);
        max_projection = std::max(max_projection, proj);
    }

    for (VerletParticle* particle : particles) {
        float proj = particle->curr_position.dot_product(normal);
        if (proj >= max_projection - 0.1f) {
            colliding_particles.push_back(particle);
        }
    }
    return colliding_particles;
}
