#pragma once

#include "material.hpp"
#include "objects.hpp"
#include "vector.hpp"
#include <memory>
#include <string>
#include <vector>

namespace render {

/**
 * @struct RenderConfig
 * @brief Render settings read from a configuration file.
 */
struct RenderConfig {
  int aspect_ratio_w{16};
  int aspect_ratio_h{9};
  int image_width{1'920};
  double gamma{2.2};
  render::vector camera_position{0.0, 0.0, -10.0};
  render::vector camera_target{0.0, 0.0, 0.0};
  render::vector camera_north{0.0, 1.0, 0.0};
  double field_of_view{90.0};
  int samples_per_pixel{20};
  int max_depth{5};
  int material_rng_seed{13};
  int ray_rng_seed{19};
  render::vector background_dark_color{0.25, 0.5, 1.0};
  render::vector background_light_color{1.0, 1.0, 1.0};
};

/**
 * @class ConfigParser
 * @brief Parser for the plain-text render configuration file.
 */
class ConfigParser {
public:
  /**
   * @param filename Path to the configuration file.
   * @return Parsed settings; keys that are not present keep their defaults.
   * @throws std::runtime_error On any I/O, syntax or validation error.
   */
  [[nodiscard]] static RenderConfig parse(std::string const &filename);
};

/**
 * @struct SceneData
 * @brief Materials and objects read from a scene file.
 *
 * Objects are stored through unique_ptr so that spheres and cylinders can
 * live in the same polymorphic list.
 */
struct SceneData {
  std::vector<std::unique_ptr<material::Material>> materials;
  std::vector<std::unique_ptr<Object>> objects;
};

/**
 * @class SceneParser
 * @brief Parser for the plain-text scene description file.
 */
class SceneParser {
public:
  /**
   * @param filename Path to the scene file.
   * @return Materials and objects of the scene.
   * @throws std::runtime_error On any I/O, syntax, validation or reference
   * error (e.g. an object using an undefined material).
   */
  [[nodiscard]] static SceneData parse(std::string const &filename);
};

} // namespace render
