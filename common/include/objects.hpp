#pragma once

#include "hit_record.hpp"
#include "interval.hpp"
#include "ray.hpp"
#include "vector.hpp"
#include <memory>
#include <string>
#include <utility>

namespace render {

/**
 * @class Object
 * @brief Abstract base class for all scene objects.
 */
class Object {
public:
  /// @param material_name Name of the material, resolved later by the scene.
  explicit Object(std::string material_name);

  virtual ~Object() = default;

  Object(Object const &) = delete;
  Object(Object &&) = delete;
  Object &operator=(Object const &) = delete;
  Object &operator=(Object &&) = delete;

  /**
   * @brief Ray-object intersection test.
   * @param r Ray.
   * @param ray_t Valid range of the ray parameter.
   * @param rec Filled with the intersection data when a hit is found.
   * @return true if the ray hits the object inside @p ray_t.
   */
  [[nodiscard]] virtual bool hit(ray const &r, Interval ray_t,
                                 HitRecord &rec) const = 0;

  [[nodiscard]] std::string const &get_material_name() const {
    return material_name_;
  }

  void set_material(std::shared_ptr<material::Material> mat) {
    material_ = std::move(mat);
  }

  [[nodiscard]] std::shared_ptr<material::Material> get_material() const {
    return material_;
  }

private:
  std::string material_name_;
  std::shared_ptr<material::Material> material_;
};

/**
 * @class Sphere
 * @brief Sphere defined by its center and radius.
 */
class Sphere : public Object {
public:
  Sphere(std::string material_name, vector const &center, double radius);

  [[nodiscard]] bool hit(ray const &r, Interval ray_t,
                         HitRecord &rec) const override;

  [[nodiscard]] vector const &get_center() const { return center_; }

  [[nodiscard]] double get_radius() const { return radius_; }

private:
  vector center_;
  double radius_;
};

/**
 * @class Cylinder
 * @brief Capped cylinder defined by its center, radius and axis vector.
 *
 * The length of the axis vector is the height of the cylinder.
 */
class Cylinder : public Object {
public:
  Cylinder(std::string material_name, vector const &center, double radius,
           vector const &axis);

  [[nodiscard]] bool hit(ray const &r, Interval ray_t,
                         HitRecord &rec) const override;

  [[nodiscard]] vector const &get_center() const { return center_; }

  [[nodiscard]] double get_radius() const { return radius_; }

  [[nodiscard]] vector const &get_axis() const { return axis_; }

private:
  vector center_;
  double radius_;
  vector axis_;

  // Precomputed in the constructor to speed up hit().
  vector axis_normalized_;
  double half_height_;

  /// Component of @p v perpendicular to the cylinder axis.
  [[nodiscard]] vector perp_component(vector const &v) const;

  /// Tests the lateral surface; updates @p t_hit if a closer hit is found.
  [[nodiscard]] bool check_lateral_hit(ray const &r, Interval ray_t,
                                       double &t_hit) const;

  /// Tests both caps; updates @p rec and @p t_hit if a closer hit is found.
  [[nodiscard]] bool check_caps_hit(ray const &r, Interval ray_t,
                                    HitRecord &rec, double &t_hit) const;
};

} // namespace render
