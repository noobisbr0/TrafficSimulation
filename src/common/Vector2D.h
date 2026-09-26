#pragma once

#include <cmath>

struct Vector2D {
    double x{0.0};
    double y{0.0};

    constexpr Vector2D() = default;
    constexpr Vector2D(double xCoord, double yCoord) : x(xCoord), y(yCoord) {}

    [[nodiscard]] constexpr Vector2D operator+(const Vector2D& other) const {
        return {x + other.x, y + other.y};
    }

    [[nodiscard]] constexpr Vector2D operator-(const Vector2D& other) const {
        return {x - other.x, y - other.y};
    }

    [[nodiscard]] constexpr Vector2D operator*(double scalar) const {
        return {x * scalar, y * scalar};
    }

    [[nodiscard]] double length() const {
        return std::sqrt(x * x + y * y);
    }
};