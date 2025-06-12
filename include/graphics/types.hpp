#pragma once

#include <cmath>
namespace Math {
    
    template<typename T>
    struct Vec2 {
            T x, y;
            Vec2(T x = 0, T y = 0) : x(x), y(y) {}
            Vec2 operator+(const Vec2& other) const {
                return Vec2(x + other.x, y + other.y);
            }

            Vec2 operator-(const Vec2& other) const {
                return Vec2(x - other.x, y - other.y);
            }

            Vec2 operator*(T scalar) const {
                return Vec2(x * scalar, y * scalar);
            }

            Vec2 operator/(T scalar) const {
                return Vec2(x / scalar, y / scalar);
            }

            T dot_product(const Vec2& other) const {
                return T(x * other.x + y * other.y);
            }

            friend inline Vec2 operator*(T scalar, const Vec2 vec) {
                return vec * scalar;
            }

            Vec2 get_perpendicular() const {
                return Vec2(-y, x);
            }

            Vec2 normalize() {
                double mag_sq = x * x + y * y;
                if (mag_sq == 0.0) {
                    return Vec2(0.0, 0.0);
                }

                T inv_mag = 1.0 / std::sqrt(mag_sq);

                return Vec2(x * inv_mag, y * inv_mag);
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
