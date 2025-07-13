#include "physics/square.hpp"
#include "shape.hpp"
#include "structures.hpp"

Square::Square(Math::Vec2<float> pos, float mass, float side_length, bool is_static, bool is_player)
    : Shape(is_static, mass, is_player),
      side_length(side_length),
      m_vertices{VerletParticle(pos, mass / 4, is_static),
                 VerletParticle(pos + Math::Vec2<float>(side_length, 0), mass / 4, is_static),
                 VerletParticle(pos + Math::Vec2<float>(side_length, side_length), mass / 4, is_static),
                 VerletParticle(pos + Math::Vec2<float>(0, side_length), mass / 4, is_static)},
      m_constraints{
          Constraint(0, 1, side_length),
          Constraint(1, 2, side_length),
          Constraint(2, 3, side_length),
          Constraint(3, 0, side_length),

          // Diagonal constrainsts
          Constraint(0, 2, sqrt(2 * (side_length * side_length))),
          Constraint(1, 3, sqrt(2 * (side_length * side_length))),
      } {}

// Is this how this is done?
Square::Square(Square&& other) noexcept
    : Shape(other.is_static, other.mass, other.is_player),
      m_vertices(std::move(other.m_vertices)),
      m_constraints(std::move(other.m_constraints)) {
    side_length = other.side_length;
    is_static = other.is_static;
    mass = other.mass;
}

Square& Square::operator=(Square&& other) noexcept {
    if (this != &other) {
        m_vertices = std::move(other.m_vertices);
        m_constraints = std::move(other.m_constraints);
        side_length = other.side_length;
        is_static = other.is_static;
        mass = other.mass;
    }
    return *this;
}

Math::Vec2<float> Square::get_curr_position() {
    return m_vertices[0].curr_position;
}

Math::Vec2<float> Square::get_center_position() {
    float cx = (m_vertices[0].curr_position.x + m_vertices[1].curr_position.x + m_vertices[2].curr_position.x +
                m_vertices[3].curr_position.x) /
               4;
    float cy = (m_vertices[0].curr_position.y + m_vertices[1].curr_position.y + m_vertices[2].curr_position.y +
                m_vertices[3].curr_position.y) /
               4;
    return Math::Vec2<float>(cx, cy);
}

float Square::get_max_side_length() {
    return side_length;
}

ArrayRef<VerletParticle> Square::get_particles() {
    return {m_vertices.data(), 4};
}

ArrayRef<Constraint> Square::get_constraints() {
    return {m_constraints.data(), 6};
}

std::vector<Math::Vec2<float>> Square::get_axes() {
    std::vector<Math::Vec2<float>> axes;
    for (int i = 0; i < 2; ++i) {
        VerletParticle p1 = m_vertices[i];
        VerletParticle p2 = m_vertices[i + 1];

        Math::Vec2<float> edge = p1.curr_position - p2.curr_position;
        Math::Vec2<float> normal = edge.get_perpendicular().normalize();

        axes.push_back(normal);
    }
    return axes;
}

Projection Square::project(Math::Vec2<float> axis) {
    double min = m_vertices[0].curr_position.dot_product(axis);
    double max = min;
    for (VerletParticle& particle : m_vertices) {
        double p = particle.curr_position.dot_product(axis);
        min = std::min(min, p);
        max = std::max(max, p);
    }
    Projection proj = Projection(min, max);
    return proj;
}

void Square::apply_force(Math::Vec2<float> f) {
    for (VerletParticle& p : m_vertices) {
        p.apply_force(f);
    }
}

float Square::get_angle() {
    Math::Vec2<float> origin = m_vertices[0].curr_position;
    Math::Vec2<float> other = m_vertices[1].curr_position;

    Math::Vec2<float> diff = other - origin;
    float angle = std::atan((-diff.y / diff.x)) * (180.0f / M_PI);
    if (diff.x < 0) {
        angle += 180;
    }
    return angle;
}

void Square::check_to_enable_player_jump(ArrayRef<VerletParticle>) {}
