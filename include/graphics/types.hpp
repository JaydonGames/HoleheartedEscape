#pragma once

#include <cmath>
namespace Render {
    struct Vec2 {
            int x, y;
    };

    struct Vector2D {
            float x, y;
            Vector2D(float x = 0, float y = 0) : x(x), y(y) {}
            Vector2D operator+(const Vector2D& other) const {
                return Vector2D(x + other.x, y + other.y);
            }

            Vector2D operator-(const Vector2D& other) const {
                return Vector2D(x - other.x, y - other.y);
            }

            Vector2D operator*(float scalar) const {
                return Vector2D(x * scalar, y * scalar);
            }

            Vector2D operator/(float scalar) const {
                return Vector2D(x / scalar, y / scalar);
            }

            float dot_product(const Vector2D& other) const {
                return float(x * other.x + y * other.y);
            }

            friend inline Vector2D operator*(float scalar, const Vector2D vec) {
                return vec * scalar;
            }

            Vector2D get_perpendicular() const {
                return Vector2D(-y, x);
            }

            Vector2D normalize() {
                double mag_sq = x * x + y * y;
                if (mag_sq == 0.0) {
                    return Vector2D(0.0, 0.0);
                }

                float inv_mag = 1.0 / std::sqrt(mag_sq);

                return Vector2D(x * inv_mag, y * inv_mag);
            }
    };

    struct Color {
            float r, g, b;
    };

    struct Vec4 {
            int x, y, z, w;
    };

    struct Rect {
            int x, y, w, h;
    };
}  // namespace Render
