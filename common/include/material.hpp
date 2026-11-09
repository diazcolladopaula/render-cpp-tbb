#pragma once

#include "hit_record.hpp"
#include "ray.hpp"
#include "vector.hpp"
#include <random>
#include <string>
#include <utility>

namespace render::material {

/**
 * @struct ScatterResult
 * @brief Outcome of a scattering event.
 */
struct ScatterResult {
  vector attenuation; ///< Color attenuation applied to the scattered ray
  ray scattered;      ///< Outgoing ray
  bool did_scatter{false};
};

/**
 * @class Material
 * @brief Abstract base class for all materials.
 */
class Material {
public:
  explicit Material(std::string name) : name_{std::move(name)} {}

  virtual ~Material() = default;

  Material(Material const &) = delete;
  Material(Material &&) = delete;
  Material &operator=(Material const &) = delete;
  Material &operator=(Material &&) = delete;

  [[nodiscard]] std::string const &get_name() const { return name_; }

  /**
   * @brief Computes how an incoming ray scatters off the surface.
   * @param r_in Incident ray.
   * @param rec Intersection record.
   * @param rng Per-thread random number generator.
   */
  [[nodiscard]] virtual ScatterResult scatter(ray const &r_in,
                                              HitRecord const &rec,
                                              std::mt19937_64 &rng) const = 0;

private:
  std::string name_;
};

/**
 * @class Matte
 * @brief Diffuse (Lambertian) material.
 */
class Matte : public Material {
public:
  Matte(std::string name, vector const &albedo);

  [[nodiscard]] ScatterResult scatter(ray const &r_in, HitRecord const &rec,
                                      std::mt19937_64 &rng) const override;

private:
  vector albedo_;
};

/**
 * @class Metal
 * @brief Reflective material with optional roughness (fuzz, clamped to 1).
 */
class Metal : public Material {
public:
  Metal(std::string name, vector const &albedo, double fuzziness);

  [[nodiscard]] ScatterResult scatter(ray const &r_in, HitRecord const &rec,
                                      std::mt19937_64 &rng) const override;

private:
  vector albedo_;
  double fuzziness_;
};

/**
 * @class Refractive
 * @brief Dielectric material (glass, water, ...) with a given refraction index.
 */
class Refractive : public Material {
public:
  Refractive(std::string name, double index_of_refraction);

  [[nodiscard]] ScatterResult scatter(ray const &r_in, HitRecord const &rec,
                                      std::mt19937_64 &rng) const override;

private:
  double index_of_refraction_;
};

} // namespace render::material
