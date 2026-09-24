#include "Vector2D.hpp"

#include <cassert>
#include <cmath>

Vector2D Vector2D::normalized() const {
    const float magnitude = length();
    assert(magnitude > EPSILON);

    return *this / magnitude;
}

void Vector2D::normalize() {
    const float magnitude = length();
    assert(magnitude > EPSILON);

    x /= magnitude;
    y /= magnitude;
}

bool Vector2D::equals(const Vector2D& rhs, float tolerance) const noexcept {
    return std::abs(x - rhs.x) <= tolerance && std::abs(y - rhs.y) <= tolerance;
}

Vector2D Vector2D::operator+(const Vector2D& rhs) const noexcept {
    return Vector2D{x + rhs.x, y + rhs.y};
}

Vector2D Vector2D::operator-(const Vector2D& rhs) const noexcept {
    return Vector2D{x - rhs.x, y - rhs.y};
}

Vector2D Vector2D::operator*(float scalar) const noexcept {
    return Vector2D{x * scalar, y * scalar};
}

Vector2D Vector2D::operator/(float scalar) const {
    assert(std::abs(scalar) > EPSILON);

    return Vector2D{x / scalar, y / scalar};
}

Vector2D& Vector2D::operator+=(const Vector2D& rhs) noexcept {
    x += rhs.x;
    y += rhs.y;
    return *this;
}

Vector2D& Vector2D::operator-=(const Vector2D& rhs) noexcept {
    x -= rhs.x;
    y -= rhs.y;
    return *this;
}

Vector2D& Vector2D::operator*=(float scalar) noexcept {
    x *= scalar;
    y *= scalar;
    return *this;
}

Vector2D& Vector2D::operator/=(float scalar) {
    assert(std::abs(scalar) > EPSILON);

    x /= scalar;
    y /= scalar;
    return *this;
}

Vector2D operator*(float scalar, const Vector2D& vec) noexcept {
    return vec * scalar;
}