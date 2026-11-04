#include "ray.hpp"
#include "vector.hpp"
#include <iostream>

namespace render {

ray::ray(vector const &origin, vector const &direction)
    : origin_{origin}, direction_{direction} {}

vector ray::at(double lambda) const { return origin_ + (direction_ * lambda); }

std::ostream &operator<<(std::ostream &os, ray const &r) {
  os << "ray(origin: " << r.origin() << ", direction: " << r.direction() << ")";
  return os;
}

} // namespace render
