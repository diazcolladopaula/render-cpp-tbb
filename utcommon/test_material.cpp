#include "material.hpp"

#include <random>
#include <string>

#include <gtest/gtest.h>

#include "hit_record.hpp"
#include "ray.hpp"
#include "vector.hpp"

TEST(TestMaterial, MatteConstructor) {
  render::material::Matte const mat{"red_matte", render::vector{0.8, 0.2, 0.2}};

  EXPECT_EQ(mat.get_name(), "red_matte");
}

TEST(TestMaterial, MatteScatter) {
  render::material::Matte const mat{"test_matte",
                                    render::vector{0.5, 0.5, 0.5}};

  render::ray const r_in{render::vector{0.0, 10.0, 0.0},
                         render::vector{0.0, -1.0, 0.0}};

  render::HitRecord rec;
  rec.point = render::vector{0.0, 0.0, 0.0};
  rec.normal = render::vector{0.0, 1.0, 0.0};
  rec.t = 10.0;
  rec.front_face = true;

  std::mt19937_64 rng(12'345);
  auto const result = mat.scatter(r_in, rec, rng);

  EXPECT_TRUE(result.did_scatter);
  EXPECT_DOUBLE_EQ(result.attenuation.x(), 0.5);
  EXPECT_DOUBLE_EQ(result.attenuation.y(), 0.5);
  EXPECT_DOUBLE_EQ(result.attenuation.z(), 0.5);

  EXPECT_DOUBLE_EQ(result.scattered.origin().x(), 0.0);
  EXPECT_DOUBLE_EQ(result.scattered.origin().y(), 0.0);
  EXPECT_DOUBLE_EQ(result.scattered.origin().z(), 0.0);
}

TEST(TestMaterial, MetalConstructor) {
  render::material::Metal const mat{"silver", render::vector{0.9, 0.9, 0.9},
                                    0.1};

  EXPECT_EQ(mat.get_name(), "silver");
}

TEST(TestMaterial, MetalScatterNoFuzz) {
  render::material::Metal const mat{"mirror", render::vector{1.0, 1.0, 1.0},
                                    0.0};

  render::ray const r_in{render::vector{0.0, 10.0, 0.0},
                         render::vector{0.0, -1.0, 0.0}};

  render::HitRecord rec;
  rec.point = render::vector{0.0, 0.0, 0.0};
  rec.normal = render::vector{0.0, 1.0, 0.0};
  rec.t = 10.0;
  rec.front_face = true;

  std::mt19937_64 rng(12'345);
  auto const result = mat.scatter(r_in, rec, rng);

  EXPECT_TRUE(result.did_scatter);
  EXPECT_DOUBLE_EQ(result.attenuation.x(), 1.0);
  EXPECT_DOUBLE_EQ(result.attenuation.y(), 1.0);
  EXPECT_DOUBLE_EQ(result.attenuation.z(), 1.0);

  EXPECT_GT(result.scattered.direction().y(), 0.0);
}

TEST(TestMaterial, MetalScatterWithFuzz) {
  render::material::Metal const mat{"fuzzy_metal",
                                    render::vector{0.8, 0.8, 0.8}, 0.5};

  render::ray const r_in{render::vector{0.0, 10.0, 0.0},
                         render::vector{0.0, -1.0, 0.0}};

  render::HitRecord rec;
  rec.point = render::vector{0.0, 0.0, 0.0};
  rec.normal = render::vector{0.0, 1.0, 0.0};
  rec.t = 10.0;
  rec.front_face = true;

  std::mt19937_64 rng(12'345);
  auto const result = mat.scatter(r_in, rec, rng);

  EXPECT_TRUE(result.did_scatter);
  EXPECT_GT(result.scattered.direction().y(), 0.0);
}

TEST(TestMaterial, RefractiveConstructor) {
  render::material::Refractive const mat{"glass", 1.5};

  EXPECT_EQ(mat.get_name(), "glass");
}

TEST(TestMaterial, RefractiveScatter) {
  render::material::Refractive const mat{"glass", 1.5};

  render::ray const r_in{render::vector{0.0, 10.0, 0.0},
                         render::vector{0.0, -1.0, 0.0}};

  render::HitRecord rec;
  rec.point = render::vector{0.0, 0.0, 0.0};
  rec.normal = render::vector{0.0, 1.0, 0.0};
  rec.t = 10.0;
  rec.front_face = true;

  std::mt19937_64 rng(12'345);
  auto const result = mat.scatter(r_in, rec, rng);

  EXPECT_TRUE(result.did_scatter);
  EXPECT_DOUBLE_EQ(result.attenuation.x(), 1.0);
  EXPECT_DOUBLE_EQ(result.attenuation.y(), 1.0);
  EXPECT_DOUBLE_EQ(result.attenuation.z(), 1.0);
}
