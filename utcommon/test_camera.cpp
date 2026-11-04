#include "camera.hpp"

#include <cstdlib>
#include <memory>
#include <vector>

#include <gtest/gtest.h>

#include "objects.hpp"
#include "parsers.hpp"

TEST(TestCamera, ConstructorBasic) {
  render::RenderConfig config;
  config.image_width = 800;
  config.aspect_ratio_w = 16;
  config.aspect_ratio_h = 9;
  config.camera_position = render::vector{0.0, 0.0, 5.0};
  config.camera_target = render::vector{0.0, 0.0, 0.0};
  config.camera_north = render::vector{0.0, 1.0, 0.0};
  config.field_of_view = 90.0;
  config.samples_per_pixel = 10;
  config.max_depth = 5;
  config.ray_rng_seed = 12'345;
  config.material_rng_seed = 67'890;
  config.gamma = 2.2;
  config.background_light_color = render::vector{1.0, 1.0, 1.0};
  config.background_dark_color = render::vector{0.5, 0.7, 1.0};

  EXPECT_NO_THROW(render::Camera const camera(config));
}

TEST(TestCamera, RenderSmallImage) {
  render::RenderConfig config;
  config.image_width = 10;
  config.aspect_ratio_w = 2;
  config.aspect_ratio_h = 1;
  config.camera_position = render::vector{0.0, 0.0, 5.0};
  config.camera_target = render::vector{0.0, 0.0, 0.0};
  config.camera_north = render::vector{0.0, 1.0, 0.0};
  config.field_of_view = 90.0;
  config.samples_per_pixel = 1;
  config.max_depth = 1;
  config.ray_rng_seed = 12'345;
  config.material_rng_seed = 67'890;
  config.gamma = 2.2;
  config.background_light_color = render::vector{1.0, 1.0, 1.0};
  config.background_dark_color = render::vector{0.5, 0.7, 1.0};

  render::Camera const camera(config);

  std::vector<std::unique_ptr<render::Object>> const objects;

  std::vector<int> const image_data = camera.render(objects);

  int const expected_size = config.image_width * 5 * 3;
  EXPECT_EQ(static_cast<int>(image_data.size()), expected_size);

  for (int const val : image_data) {
    EXPECT_GE(val, 0);
    EXPECT_LE(val, 255);
  }
}

TEST(TestCamera, RenderWithObject) {
  render::RenderConfig config;
  config.image_width = 10;
  config.aspect_ratio_w = 1;
  config.aspect_ratio_h = 1;
  config.camera_position = render::vector{0.0, 0.0, 5.0};
  config.camera_target = render::vector{0.0, 0.0, 0.0};
  config.camera_north = render::vector{0.0, 1.0, 0.0};
  config.field_of_view = 90.0;
  config.samples_per_pixel = 1;
  config.max_depth = 1;
  config.ray_rng_seed = 12'345;
  config.material_rng_seed = 67'890;
  config.gamma = 2.2;
  config.background_light_color = render::vector{1.0, 1.0, 1.0};
  config.background_dark_color = render::vector{0.5, 0.7, 1.0};

  render::Camera const camera(config);

  std::vector<std::unique_ptr<render::Object>> objects;
  objects.push_back(std::make_unique<render::Sphere>(
      "mat", render::vector{0.0, 0.0, 0.0}, 1.0));

  std::vector<int> const image_data = camera.render(objects);

  int const expected_size = config.image_width * 10 * 3;
  EXPECT_EQ(static_cast<int>(image_data.size()), expected_size);
}
