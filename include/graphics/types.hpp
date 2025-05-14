#pragma once

namespace Render {
    struct Vec2 {
        int x, y;
    };

    struct Vector2D {
        float x, y;
        Vector2D(float x = 0, float y = 0) : x(x), y(y) {}
        Vector2D operator +(const Vector2D& other) const {
            return Vector2D(x + other.x, y + other.y);
        }

        Vector2D operator -(const Vector2D& other) const {
            return Vector2D(x - other.x, y- other.y);
        }

        Vector2D operator *(float scalar) const {
            return Vector2D(x * scalar, y * scalar);
        }

        Vector2D operator /(float scalar) const {
            return Vector2D(x / scalar, y / scalar);
        }

        float dot_product(const Vector2D& other) const{
            return float(x * other.x + y * other.y);
        }
    };

    inline Vector2D operator *(float scalar, const Vector2D vec){
    return vec * scalar;
    };

    struct Vec3 {
        int x, y, z;
    };

    struct Vec4 {
        int x, y, z, w;
    };

    struct Rect {
        float x, y;
        int w, h;
    };
}
