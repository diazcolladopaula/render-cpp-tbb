#include "material.hpp"
#include "hit_record.hpp"
#include "ray.hpp"
#include "vector.hpp"
#include <algorithm>
#include <cmath>
#include <random>
#include <string>
#include <utility>

namespace render::material {

namespace {

// Random unit vector, obtained by rejection sampling inside the unit sphere.
vector random_unit_vector(std::mt19937_64 &rng) {
  std::uniform_real_distribution<double> dist(-1.0, 1.0);
  while (true) {
    vector const p(dist(rng), dist(rng), dist(rng));
    double const len_sq = p.magnitude_squared();

    constexpr double epsilon = 1e-9;
    if (epsilon < len_sq and len_sq <= 1.0) {
      return p / std::sqrt(len_sq);
    }
  }
}

vector reflect(vector const &v, vector const &n) {
  return v - n * (2.0 * v.dot(n));
}

// Snell's law for a unit incident vector uv and ratio etai / etat.
vector refract(vector const &uv, vector const &n, double etai_over_etat) {
  double const cos_theta = std::min(-uv.dot(n), 1.0);
  vector const r_out_perp = (uv + n * cos_theta) * etai_over_etat;
  vector const r_out_parallel =
      n * (-std::sqrt(std::abs(1.0 - r_out_perp.magnitude_squared())));
  return r_out_perp + r_out_parallel;
}

// Schlick's approximation of the Fresnel reflectance.
double reflectance(double cosine, double ref_idx) {
  double r0 = (1 - ref_idx) / (1 + ref_idx);
  r0 = r0 * r0;
  return r0 + (1 - r0) * std::pow(1 - cosine, 5);
}

} // namespace

Matte::Matte(std::string name, vector const &albedo)
    : Material(std::move(name)), albedo_{albedo} {}

ScatterResult Matte::scatter(ray const & /*r_in*/, HitRecord const &rec,
                             std::mt19937_64 &rng) const {
  vector scatter_direction = rec.normal + random_unit_vector(rng);

  // Avoid a degenerate direction when the random vector cancels the normal.
  constexpr double epsilon = 1e-9;
  if (std::abs(scatter_direction.x()) < epsilon and
      std::abs(scatter_direction.y()) < epsilon and
      std::abs(scatter_direction.z()) < epsilon) {
    scatter_direction = rec.normal;
  }
  return ScatterResult{.attenuation = albedo_,
                       .scattered = ray(rec.point, scatter_direction),
                       .did_scatter = true};
}

Metal::Metal(std::string name, vector const &albedo, double fuzziness)
    : Material(std::move(name)), albedo_{albedo},
      fuzziness_{std::min(fuzziness, 1.0)} {}

ScatterResult Metal::scatter(ray const &r_in, HitRecord const &rec,
                             std::mt19937_64 &rng) const {
  vector const reflected = reflect(r_in.direction().normalize(), rec.normal);
  vector const fuzz_vec = (fuzziness_ > 0)
                              ? (random_unit_vector(rng) * fuzziness_)
                              : vector(0, 0, 0);
  return ScatterResult{.attenuation = albedo_,
                       .scattered = ray(rec.point, reflected + fuzz_vec),
                       .did_scatter = true};
}

Refractive::Refractive(std::string name, double index_of_refraction)
    : Material(std::move(name)), index_of_refraction_{index_of_refraction} {}

ScatterResult Refractive::scatter(ray const &r_in, HitRecord const &rec,
                                  std::mt19937_64 &rng) const {
  double const ri =
      rec.front_face ? (1.0 / index_of_refraction_) : index_of_refraction_;
  vector const unit_direction = r_in.direction().normalize();
  double const cos_theta = std::min(-unit_direction.dot(rec.normal), 1.0);
  double const sin_theta = std::sqrt(1.0 - cos_theta * cos_theta);

  // Total internal reflection, or a probabilistic Fresnel reflection.
  bool const cannot_refract = ri * sin_theta > 1.0;
  std::uniform_real_distribution<double> dist(0.0, 1.0);
  vector direction;
  if (cannot_refract or reflectance(cos_theta, ri) > dist(rng)) {
    direction = reflect(unit_direction, rec.normal);
  } else {
    direction = refract(unit_direction, rec.normal, ri);
  }
  return ScatterResult{.attenuation = vector(1.0, 1.0, 1.0),
                       .scattered = ray(rec.point, direction),
                       .did_scatter = true};
}

} // namespace render::material
