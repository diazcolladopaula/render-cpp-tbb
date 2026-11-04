#pragma once

#include <cmath>
#include <iostream>

namespace render {

/**
 * @class vector
 * @brief 3D vector (also used for points and RGB colors).
 */
class vector {
public:
  /// Creates the zero vector.
  vector() = default;

  vector(double cx, double cy, double cz) : x_{cx}, y_{cy}, z_{cz} {}

  [[nodiscard]] double x() const { return x_; }

  [[nodiscard]] double y() const { return y_; }

  [[nodiscard]] double z() const { return z_; }

  /// Euclidean length.
  [[nodiscard]] double magnitude() const;

  /// Squared length (avoids the square root).
  [[nodiscard]] double magnitude_squared() const;

  [[nodiscard]] double dot(vector const &other) const;

  [[nodiscard]] vector cross(vector const &other) const;

  /// Returns a unit-length copy; a near-zero vector is returned unchanged.
  [[nodiscard]] vector normalize() const;

  [[nodiscard]] vector operator-() const;

  vector &operator+=(vector const &other);

  vector &operator-=(vector const &other);

  vector &operator*=(double scalar);

  vector &operator/=(double scalar);

private:
  double x_{0.0};
  double y_{0.0};
  double z_{0.0};
};

[[nodiscard]] vector operator+(vector const &lhs, vector const &rhs);

[[nodiscard]] vector operator-(vector const &lhs, vector const &rhs);

/// Component-wise product.
[[nodiscard]] vector operator*(vector const &lhs, vector const &rhs);

[[nodiscard]] vector operator*(vector const &v, double scalar);

[[nodiscard]] vector operator*(double scalar, vector const &v);

[[nodiscard]] vector operator/(vector const &v, double scalar);

/// Stream output, mainly for debugging and test diagnostics.
std::ostream &operator<<(std::ostream &os, vector const &v);

} // namespace render
