#include "vector.hpp"
#include <cmath>
#include <iostream>

namespace render {

double vector::magnitude() const {
  return std::sqrt(x_ * x_ + y_ * y_ + z_ * z_);
}

double vector::magnitude_squared() const { return x_ * x_ + y_ * y_ + z_ * z_; }

double vector::dot(vector const &other) const {
  return x_ * other.x_ + y_ * other.y_ + z_ * other.z_;
}

vector vector::cross(vector const &other) const {
  return vector{y_ * other.z_ - z_ * other.y_, z_ * other.x_ - x_ * other.z_,
                x_ * other.y_ - y_ * other.x_};
}

vector vector::normalize() const {
  double const mag = magnitude();
  constexpr double epsilon = 1e-10;
  if (mag < epsilon) {
    return *this;
  }
  return *this / mag;
}

vector vector::operator-() const { return vector{-x_, -y_, -z_}; }

vector &vector::operator+=(vector const &other) {
  x_ += other.x_;
  y_ += other.y_;
  z_ += other.z_;
  return *this;
}

vector &vector::operator-=(vector const &other) {
  x_ -= other.x_;
  y_ -= other.y_;
  z_ -= other.z_;
  return *this;
}

vector &vector::operator*=(double scalar) {
  x_ *= scalar;
  y_ *= scalar;
  z_ *= scalar;
  return *this;
}

vector &vector::operator/=(double const scalar) {
  return *this *= (1.0 / scalar);
}

vector operator+(vector const &lhs, vector const &rhs) {
  return vector(lhs) += rhs;
}

vector operator-(vector const &lhs, vector const &rhs) {
  return vector(lhs) -= rhs;
}

// Component-wise product (used to combine colors).
vector operator*(vector const &lhs, vector const &rhs) {
  return vector{lhs.x() * rhs.x(), lhs.y() * rhs.y(), lhs.z() * rhs.z()};
}

vector operator*(vector const &v, double scalar) { return vector(v) *= scalar; }

vector operator*(double scalar, vector const &v) { return v * scalar; }

vector operator/(vector const &v, double scalar) { return vector(v) /= scalar; }

std::ostream &operator<<(std::ostream &os, vector const &v) {
  os << "vector(" << v.x() << ", " << v.y() << ", " << v.z() << ")";
  return os;
}

} // namespace render
