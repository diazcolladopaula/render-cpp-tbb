#pragma once

#include "vector.hpp"
#include <memory>

namespace render {

namespace material {

class Material;

}

/**
 * @struct HitRecord
 * @brief Information about a ray-object intersection.
 */
struct HitRecord {
  vector point;          ///< Intersection point
  vector normal;         ///< Unit normal, always facing against the ray
  double t{0.0};         ///< Ray parameter at the intersection
  bool front_face{true}; ///< True if the ray hit the outer side of the surface
  std::shared_ptr<material::Material> mat_ptr; ///< Material of the hit object

  /**
   * @brief Sets @c normal so that it always opposes the incident ray.
   * @param ray_direction Direction of the incident ray.
   * @param outward_normal Unit normal pointing out of the object.
   */
  void set_face_normal(vector const &ray_direction,
                       vector const &outward_normal) {
    front_face = ray_direction.dot(outward_normal) < 0;
    normal = front_face ? outward_normal : -outward_normal;
  }
};

} // namespace render
