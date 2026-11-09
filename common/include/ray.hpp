#ifndef RENDER_RAY_HPP
#define RENDER_RAY_HPP

#include "vector.hpp"
#include <iostream>

namespace render {

/**
 * @class ray
 * @brief Half-line defined by an origin and a direction:
 * P(t) = origin + t * direction.
 */
class ray {
public:
  /// Creates a ray at the origin with a zero direction.
  ray() = default;

  ray(vector const &origin, vector const &direction);

  [[nodiscard]] vector origin() const { return origin_; }

  [[nodiscard]] vector direction() const { return direction_; }

  /// Point along the ray at parameter @p lambda.
  [[nodiscard]] vector at(double lambda) const;

private:
  vector origin_;
  vector direction_;
};

std::ostream &operator<<(std::ostream &os, ray const &r);

} // namespace render

#endif
