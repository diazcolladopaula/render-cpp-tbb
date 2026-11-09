#include "camera.hpp"
#include "hit_record.hpp"
#include "interval.hpp"
#include "objects.hpp"
#include "parsers.hpp"
#include "ray.hpp"
#include <algorithm>
#include <atomic>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <memory>
#include <numbers>
#include <random>
#include <vector>

// oneTBB headers
#include <oneapi/tbb/blocked_range2d.h>
#include <oneapi/tbb/enumerable_thread_specific.h>
#include <oneapi/tbb/parallel_for.h>
#include <oneapi/tbb/partitioner.h>

namespace render {

Camera::Camera(RenderConfig const &config)
    : config_{config},
      base_seed_{static_cast<std::uint64_t>(config.ray_rng_seed) *
                     std::uint64_t{1'000'000} +
                 static_cast<std::uint64_t>(config.material_rng_seed)} {
  initialize();
}

void Camera::initialize() {
  image_height_ = static_cast<int>(
      config_.image_width / (static_cast<double>(config_.aspect_ratio_w) /
                             static_cast<double>(config_.aspect_ratio_h)));
  image_height_ = std::max(1, image_height_);

  camera_center_ = config_.camera_position;

  double const focal_length =
      (config_.camera_position - config_.camera_target).magnitude();
  constexpr double pi = std::numbers::pi;
  double const theta = config_.field_of_view * pi / 180.0;
  double const h = std::tan(theta / 2.0);
  double const viewport_height = 2.0 * h * focal_length;
  double const viewport_width =
      viewport_height * (static_cast<double>(config_.image_width) /
                         static_cast<double>(image_height_));

  // Orthonormal camera basis
  vector const w =
      (config_.camera_position - config_.camera_target).normalize();
  vector const u = config_.camera_north.cross(w).normalize();
  vector const v = w.cross(u);

  // Viewport edge vectors and per-pixel steps
  viewport_u_ = u * viewport_width;
  viewport_v_ = v * (-viewport_height);

  pixel_delta_u_ = viewport_u_ / static_cast<double>(config_.image_width);
  pixel_delta_v_ = viewport_v_ / static_cast<double>(image_height_);

  viewport_upper_left_ =
      camera_center_ - w * focal_length - viewport_u_ / 2.0 - viewport_v_ / 2.0;

  pixel00_loc_ = viewport_upper_left_ + (pixel_delta_u_ + pixel_delta_v_) * 0.5;
}

ray Camera::get_ray(int i, int j, std::mt19937_64 &rng) const {
  std::uniform_real_distribution<double> dist(-0.5, 0.5);
  // Random sub-pixel offset for antialiasing
  double const offset_x = dist(rng);
  double const offset_y = dist(rng);
  vector const pixel_sample =
      pixel00_loc_ + pixel_delta_u_ * (static_cast<double>(j) + offset_x) +
      pixel_delta_v_ * (static_cast<double>(i) + offset_y);
  return ray{camera_center_, pixel_sample - camera_center_};
}

bool Camera::hit_objects(ray const &r, Interval ray_t,
                         std::vector<std::unique_ptr<Object>> const &objects,
                         HitRecord &rec) {
  HitRecord temp_rec;
  bool hit_anything = false;
  double closest_so_far = ray_t.max;
  for (auto const &object : objects) {
    if (object->hit(r, Interval(ray_t.min, closest_so_far), temp_rec)) {
      hit_anything = true;
      closest_so_far = temp_rec.t;
      rec = temp_rec;
    }
  }
  return hit_anything;
}

vector Camera::background_color(ray const &r) const {
  vector const unit_direction = r.direction().normalize();
  double const a = 0.5 * (unit_direction.y() + 1.0);
  // Linear blend between the two background colors
  return config_.background_light_color * (1.0 - a) +
         config_.background_dark_color * a;
}

vector Camera::ray_color(ray const &r, int depth,
                         std::vector<std::unique_ptr<Object>> const &objects,
                         std::mt19937_64 &rng) const {
  if (depth <= 0) {
    return vector{0, 0, 0};
  }
  HitRecord rec;
  if (Camera::hit_objects(
          r, Interval(0.001, std::numeric_limits<double>::infinity()), objects,
          rec)) {
    if (rec.mat_ptr) {
      auto const result = rec.mat_ptr->scatter(r, rec, rng);
      if (result.did_scatter) {
        vector const color =
            ray_color(result.scattered, depth - 1, objects, rng);
        return result.attenuation * color;
      }
    }
    // Absorbed by the surface
    return vector{0, 0, 0};
  }
  return background_color(r);
}

void Camera::apply_gamma_and_store(vector const &color, double gamma_correction,
                                   std::vector<int> &image_data,
                                   std::size_t base_index) {
  auto to_byte = [](double value) -> int {
    value = std::clamp(value, 0.0, 0.9999999);
    return static_cast<int>(256 * value);
  };
  vector const corrected_color = vector(std::pow(color.x(), gamma_correction),
                                        std::pow(color.y(), gamma_correction),
                                        std::pow(color.z(), gamma_correction));
  image_data[base_index] = to_byte(corrected_color.x());
  image_data[base_index + 1] = to_byte(corrected_color.y());
  image_data[base_index + 2] = to_byte(corrected_color.z());
}

vector
Camera::trace_ray_sample(ray const &r,
                         std::vector<std::unique_ptr<Object>> const &objects,
                         std::mt19937_64 &rng, bool &did_hit) const {
  HitRecord rec;
  did_hit =
      hit_objects(r, Interval(0.001, std::numeric_limits<double>::infinity()),
                  objects, rec);

  if (did_hit) {
    if (rec.mat_ptr) {
      auto const result = rec.mat_ptr->scatter(r, rec, rng);
      if (result.did_scatter) {
        vector const bounced_color =
            ray_color(result.scattered, config_.max_depth - 1, objects, rng);
        return result.attenuation * bounced_color;
      }
    }
    return vector{0, 0, 0};
  }
  return background_color(r);
}

vector
Camera::compute_pixel_color(int j, int i,
                            std::vector<std::unique_ptr<Object>> const &objects,
                            std::mt19937_64 &rng) const {
  // Early exit: if none of the first samples hits an object, the pixel is
  // treated as background and the remaining samples are skipped.
  constexpr int initial_samples_check = 4;
  bool const apply_optimization =
      config_.samples_per_pixel >= (initial_samples_check + 1);
  vector pixel_color_sum{0, 0, 0};
  bool any_hit_in_first_samples = false;
  int samples_taken = 0;
  for (int s = 0; s < config_.samples_per_pixel; ++s) {
    samples_taken++;
    bool did_hit = false;
    pixel_color_sum +=
        trace_ray_sample(get_ray(j, i, rng), objects, rng, did_hit);
    if (apply_optimization) {
      if (s < initial_samples_check and did_hit) {
        any_hit_in_first_samples = true;
      }
      if (s == (initial_samples_check - 1) and !any_hit_in_first_samples) {
        break;
      }
    }
  }
  return pixel_color_sum * (1.0 / static_cast<double>(samples_taken));
}

int Camera::determine_effective_grain(RenderParams const &params) {
  if (params.grain_size > 0) {
    return params.grain_size;
  }
  switch (params.policy) {
  case PartitionPolicy::Simple:
    return 32;
  case PartitionPolicy::Static:
  case PartitionPolicy::Auto:
    return 1;
  }
  return 1;
}

Camera::PixelProcessor Camera::create_pixel_processor(
    std::vector<std::unique_ptr<Object>> const &objects,
    tbb::enumerable_thread_specific<std::mt19937_64> &thread_local_rng,
    std::vector<int> &image_data, double gamma_correction) const {
  return [&, this](tbb::blocked_range2d<int> const &range) {
    std::mt19937_64 &local_rng = thread_local_rng.local();

    for (int j = range.rows().begin(); j != range.rows().end(); ++j) {
      for (int i = range.cols().begin(); i != range.cols().end(); ++i) {
        vector const avg_pixel_color =
            compute_pixel_color(j, i, objects, local_rng);
        std::size_t const pixel_index =
            static_cast<std::size_t>(j) *
                static_cast<std::size_t>(config_.image_width) +
            static_cast<std::size_t>(i);
        std::size_t const base_index = pixel_index * 3;
        apply_gamma_and_store(avg_pixel_color, gamma_correction, image_data,
                              base_index);
      }
    }
  };
}

void Camera::execute_parallel_render(PixelProcessor const &processor,
                                     RenderParams const &params,
                                     int effective_grain) const {
  switch (params.policy) {
  case PartitionPolicy::Simple:
    tbb::parallel_for(
        tbb::blocked_range2d<int>(
            0, image_height_, static_cast<std::size_t>(effective_grain), 0,
            config_.image_width, static_cast<std::size_t>(effective_grain)),
        processor, tbb::simple_partitioner{});
    break;
  case PartitionPolicy::Static:
    tbb::parallel_for(
        tbb::blocked_range2d<int>(0, image_height_, 0, config_.image_width),
        processor, tbb::static_partitioner{});
    break;
  case PartitionPolicy::Auto:
  default:
    // With an explicit grain size, use it; otherwise let TBB choose.
    if (params.grain_size > 0) {
      tbb::parallel_for(
          tbb::blocked_range2d<int>(
              0, image_height_, static_cast<std::size_t>(effective_grain), 0,
              config_.image_width, static_cast<std::size_t>(effective_grain)),
          processor, tbb::auto_partitioner{});
    } else {
      tbb::parallel_for(
          tbb::blocked_range2d<int>(0, image_height_, 0, config_.image_width),
          processor, tbb::auto_partitioner{});
    }
    break;
  }
}

std::vector<int>
Camera::render(std::vector<std::unique_ptr<Object>> const &objects,
               RenderParams const &params) const {
  std::size_t const total_pixels =
      static_cast<std::size_t>(config_.image_width) *
      static_cast<std::size_t>(image_height_);
  std::vector<int> image_data(total_pixels * 3);

  double const gamma_correction = 1.0 / config_.gamma;

  // Each thread lazily creates its own generator, seeded from a unique id.
  std::atomic<std::uint64_t> thread_counter{0};
  tbb::enumerable_thread_specific<std::mt19937_64> thread_local_rng{
      [this, &thread_counter]() {
        std::uint64_t const thread_id = thread_counter.fetch_add(1);
        return std::mt19937_64{base_seed_ + thread_id * 1'000'000ULL};
      }};

  PixelProcessor const processor = create_pixel_processor(
      objects, thread_local_rng, image_data, gamma_correction);

  int const effective_grain = determine_effective_grain(params);
  execute_parallel_render(processor, params, effective_grain);
  return image_data;
}

} // namespace render
