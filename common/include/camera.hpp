#pragma once

#include "interval.hpp"
#include "objects.hpp"
#include "parsers.hpp"
#include "ray.hpp"
#include "vector.hpp"
#include <cstdint>
#include <functional>
#include <memory>
#include <random>
#include <vector>

// oneTBB headers
#include <oneapi/tbb/blocked_range2d.h>
#include <oneapi/tbb/enumerable_thread_specific.h>
#include <oneapi/tbb/parallel_for.h>
#include <oneapi/tbb/partitioner.h>

namespace render {

/**
 * @enum PartitionPolicy
 * @brief Work-partitioning strategies, mapped to TBB partitioners.
 */
enum class PartitionPolicy { Auto, Simple, Static };

/**
 * @struct RenderParams
 * @brief Parallel rendering options.
 */
struct RenderParams {
  PartitionPolicy policy{PartitionPolicy::Auto};
  int grain_size{0}; // 0 = use the default for the selected policy
};

/**
 * @class Camera
 * @brief Generates camera rays and renders the scene in parallel.
 *
 * The pixel loop runs under tbb::parallel_for with a selectable partitioner
 * (auto, simple, static) and grain size. Each thread has its own random number
 * generator (tbb::enumerable_thread_specific).
 */
class Camera {
public:
  /**
   * @param config Render settings.
   */
  explicit Camera(RenderConfig const &config);

  /**
   * @brief Renders the scene in parallel.
   * @param objects Objects in the scene.
   * @param params Partitioning policy and grain size.
   * @return Interleaved RGB values in [0, 255], row by row.
   */
  [[nodiscard]] std::vector<int>
  render(std::vector<std::unique_ptr<Object>> const &objects,
         RenderParams const &params = RenderParams{}) const;

private:
  // Render settings
  RenderConfig config_;

  // Camera geometry, computed by initialize()
  int image_height_{0};
  vector camera_center_;
  vector viewport_u_;
  vector viewport_v_;
  vector pixel_delta_u_;
  vector pixel_delta_v_;
  vector viewport_upper_left_;
  vector pixel00_loc_;

  // Base seed from which each thread derives its own generator seed
  std::uint64_t base_seed_;

  /**
   * @brief Computes the viewport geometry from the configuration.
   */
  void initialize();

  /**
   * @brief Ray through a random point inside pixel (row i, column j).
   * @param rng Per-thread random number generator (used for antialiasing).
   */
  [[nodiscard]] ray get_ray(int i, int j, std::mt19937_64 &rng) const;

  /**
   * @brief Recursively computes the color seen along a ray.
   * @param depth Remaining number of bounces.
   */
  [[nodiscard]] vector
  ray_color(ray const &r, int depth,
            std::vector<std::unique_ptr<Object>> const &objects,
            std::mt19937_64 &rng) const;

  /**
   * @brief Finds the closest intersection among all objects.
   */
  [[nodiscard]] static bool
  hit_objects(ray const &r, Interval ray_t,
              std::vector<std::unique_ptr<Object>> const &objects,
              HitRecord &rec);

  /**
   * @brief Sky gradient color for rays that miss every object.
   */
  [[nodiscard]] vector background_color(ray const &r) const;

  /**
   * @brief Traces one sample ray and reports whether it hit any object.
   */
  [[nodiscard]] vector
  trace_ray_sample(ray const &r,
                   std::vector<std::unique_ptr<Object>> const &objects,
                   std::mt19937_64 &rng, bool &did_hit) const;

  /**
   * @brief Averages several samples for pixel (row j, column i).
   *
   * If none of the first few samples hits an object, the pixel is assumed to
   * be plain background and sampling stops early.
   *
   * @return Average color (linear, before gamma correction).
   */
  [[nodiscard]] vector
  compute_pixel_color(int j, int i,
                      std::vector<std::unique_ptr<Object>> const &objects,
                      std::mt19937_64 &rng) const;

  /**
   * @brief Applies gamma correction and stores the color as RGB in [0, 255].
   */
  static void apply_gamma_and_store(vector const &color,
                                    double gamma_correction,
                                    std::vector<int> &image_data,
                                    std::size_t base_index);

  /**
   * @brief Grain size to use: the requested one, or a default per policy.
   */
  [[nodiscard]] static int
  determine_effective_grain(RenderParams const &params);

  /**
   * @brief Callable that shades a block of pixels, run by parallel_for.
   */
  using PixelProcessor = std::function<void(tbb::blocked_range2d<int> const &)>;

  [[nodiscard]] PixelProcessor create_pixel_processor(
      std::vector<std::unique_ptr<Object>> const &objects,
      tbb::enumerable_thread_specific<std::mt19937_64> &thread_local_rng,
      std::vector<int> &image_data, double gamma_correction) const;

  /**
   * @brief Runs parallel_for with the partitioner chosen by the policy.
   */
  void execute_parallel_render(PixelProcessor const &processor,
                               RenderParams const &params,
                               int effective_grain) const;
};

} // namespace render
