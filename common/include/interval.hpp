#pragma once

#include <limits>

namespace render {

/**
 * @class Interval
 * @brief Closed interval [min, max] used to bound valid ray parameters.
 */
class Interval {
public:
  double min;
  double max;

  /// Empty interval.
  Interval()
      : min(std::numeric_limits<double>::infinity()),
        max(-std::numeric_limits<double>::infinity()) {}

  Interval(double min_val, double max_val) : min(min_val), max(max_val) {}

  /// True if min <= x <= max.
  [[nodiscard]] bool contains(double x) const { return min <= x and x <= max; }

  /// True if min < x < max (bounds excluded).
  [[nodiscard]] bool surrounds(double x) const { return min < x and x < max; }

  static Interval const empty;
  static Interval const universe;
};

inline Interval const Interval::empty = Interval();
inline Interval const Interval::universe =
    Interval(-std::numeric_limits<double>::infinity(),
             std::numeric_limits<double>::infinity());

} // namespace render
