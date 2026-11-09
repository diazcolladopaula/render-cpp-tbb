#include "objects.hpp"
#include "hit_record.hpp"
#include "interval.hpp"
#include "ray.hpp"
#include "vector.hpp"
#include <cmath>
#include <string>
#include <utility>

namespace render {

Object::Object(std::string material_name)
    : material_name_{std::move(material_name)} {}

Sphere::Sphere(std::string material_name, vector const &center, double radius)
    : Object(std::move(material_name)), center_{center}, radius_{radius} {}

bool Sphere::hit(ray const &r, Interval ray_t, HitRecord &rec) const {
  vector const oc = center_ - r.origin();
  double const a = r.direction().magnitude_squared();
  double const h = r.direction().dot(oc);
  double const c = oc.magnitude_squared() - radius_ * radius_;
  double const discriminant = h * h - a * c;

  if (discriminant < 0) {
    return false;
  }

  double const sqrtd = std::sqrt(discriminant);
  double root = (h - sqrtd) / a;

  if (!ray_t.surrounds(root)) {
    root = (h + sqrtd) / a;
    if (!ray_t.surrounds(root)) {
      return false;
    }
  }

  rec.t = root;
  rec.point = r.at(rec.t);
  vector const outward_normal = (rec.point - center_) / radius_;
  rec.set_face_normal(r.direction(), outward_normal);
  rec.mat_ptr = get_material();

  return true;
}

Cylinder::Cylinder(std::string material_name, vector const &center,
                   double radius, vector const &axis)
    : Object(std::move(material_name)), center_{center}, radius_{radius},
      axis_{axis}, axis_normalized_{axis_.normalize()},
      half_height_{axis_.magnitude() / 2.0} {}

vector Cylinder::perp_component(vector const &v) const {
  return v - axis_normalized_ * v.dot(axis_normalized_);
}

bool Cylinder::check_lateral_hit(ray const &r, Interval ray_t,
                                 double &t_hit) const {
  vector const rc = r.origin() - center_;
  vector const rc_perp = perp_component(rc);
  vector const rd_perp = perp_component(r.direction());

  double const a = rd_perp.magnitude_squared();
  if (std::abs(a) < 1e-8) {
    return false; // Ray parallel to the axis
  }

  double const b = 2.0 * rc_perp.dot(rd_perp);
  double const c = rc_perp.magnitude_squared() - radius_ * radius_;
  double const discriminant = b * b - 4 * a * c;

  if (discriminant < 0) {
    return false;
  }

  double const sqrt_disc = std::sqrt(discriminant);
  double const t1 = (-b - sqrt_disc) / (2.0 * a);
  double const t2 = (-b + sqrt_disc) / (2.0 * a);
  bool hit = false;

  for (double const t : {t1, t2}) {
    if (t >= ray_t.min and t < t_hit) {
      vector const hit_point = r.at(t);
      vector const hit_rel = hit_point - center_;
      double const height_pos = hit_rel.dot(axis_normalized_);

      if (std::abs(height_pos) <= half_height_) {
        t_hit = t;
        hit = true;
      }
    }
  }
  return hit;
}

bool Cylinder::check_caps_hit(ray const &r, Interval ray_t, HitRecord &rec,
                              double &t_hit) const {
  bool hit_cap = false;
  for (int const sign : {-1, 1}) {
    vector const cap_center =
        center_ + axis_normalized_ * (half_height_ * sign);
    vector const normal = axis_normalized_ * sign;
    double const denom = r.direction().dot(normal);

    if (std::abs(denom) > 1e-8) {
      double const t = (cap_center - r.origin()).dot(normal) / denom;

      if (t >= ray_t.min and t < t_hit) {
        vector const hit_point = r.at(t);
        vector const to_center = hit_point - cap_center;

        if (to_center.magnitude_squared() <= radius_ * radius_) {
          t_hit = t;
          hit_cap = true;
          rec.t = t;
          rec.point = hit_point;
          rec.set_face_normal(r.direction(), normal);
          rec.mat_ptr = get_material();
        }
      }
    }
  }
  return hit_cap;
}

bool Cylinder::hit(ray const &r, Interval ray_t, HitRecord &rec) const {
  double t_hit = ray_t.max;
  HitRecord cap_rec;

  // Both checks only accept hits closer than t_hit, so if a cap is hit it is
  // necessarily closer than any lateral hit found before it.
  bool const hit_lateral = check_lateral_hit(r, ray_t, t_hit);
  bool const hit_cap = check_caps_hit(r, ray_t, cap_rec, t_hit);

  if (not hit_lateral and not hit_cap) {
    return false;
  }

  if (hit_cap) {
    rec = cap_rec;
  } else {
    rec.t = t_hit;
    rec.point = r.at(t_hit);
    // The perpendicular component has length radius_, so normalize it.
    vector const outward_normal =
        perp_component(rec.point - center_) / radius_;
    rec.set_face_normal(r.direction(), outward_normal);
    rec.mat_ptr = get_material();
  }

  return true;
}

} // namespace render
