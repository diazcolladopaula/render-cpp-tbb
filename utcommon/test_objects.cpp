#include "objects.hpp"

#include <string>

#include <gtest/gtest.h>

#include "hit_record.hpp"
#include "interval.hpp"
#include "ray.hpp"
#include "vector.hpp"

TEST(TestObjects, SphereConstructor) {
  render::Sphere const sphere{"test_material", render::vector{0.0, 0.0, 0.0},
                              1.0};

  EXPECT_EQ(sphere.get_material_name(), "test_material");
}

TEST(TestObjects, SphereHitDirect) {
  render::Sphere const sphere{"mat", render::vector{0.0, 0.0, 0.0}, 1.0};

  render::ray const r{render::vector{0.0, 5.0, 0.0},
                      render::vector{0.0, -1.0, 0.0}};

  render::HitRecord rec;
  render::Interval const ray_t{0.001, 1000.0};

  bool const hit = sphere.hit(r, ray_t, rec);

  EXPECT_TRUE(hit);
  EXPECT_DOUBLE_EQ(rec.t, 4.0);

  EXPECT_NEAR(rec.point.x(), 0.0, 1e-10);
  EXPECT_NEAR(rec.point.y(), 1.0, 1e-10);
  EXPECT_NEAR(rec.point.z(), 0.0, 1e-10);

  EXPECT_NEAR(rec.normal.x(), 0.0, 1e-10);
  EXPECT_NEAR(rec.normal.y(), 1.0, 1e-10);
  EXPECT_NEAR(rec.normal.z(), 0.0, 1e-10);
}

TEST(TestObjects, SphereNoHit) {
  render::Sphere const sphere{"mat", render::vector{0.0, 0.0, 0.0}, 1.0};

  render::ray const r{render::vector{5.0, 0.0, 0.0},
                      render::vector{0.0, 1.0, 0.0}};

  render::HitRecord rec;
  render::Interval const ray_t{0.001, 1000.0};

  bool const hit = sphere.hit(r, ray_t, rec);

  EXPECT_FALSE(hit);
}

TEST(TestObjects, SphereHitFromInside) {
  render::Sphere const sphere{"mat", render::vector{0.0, 0.0, 0.0}, 2.0};

  render::ray const r{render::vector{0.0, 0.0, 0.0},
                      render::vector{1.0, 0.0, 0.0}};

  render::HitRecord rec;
  render::Interval const ray_t{0.001, 1000.0};

  bool const hit = sphere.hit(r, ray_t, rec);

  EXPECT_TRUE(hit);
  EXPECT_DOUBLE_EQ(rec.t, 2.0);
  EXPECT_FALSE(rec.front_face);
}

TEST(TestObjects, SphereHitOutsideInterval) {
  render::Sphere const sphere{"mat", render::vector{0.0, 0.0, 0.0}, 1.0};

  render::ray const r{render::vector{0.0, 5.0, 0.0},
                      render::vector{0.0, -1.0, 0.0}};

  render::HitRecord rec;
  render::Interval const ray_t{7.0, 1000.0};

  bool const hit = sphere.hit(r, ray_t, rec);

  EXPECT_FALSE(hit);
}

TEST(TestObjects, CylinderConstructor) {
  render::Cylinder const cylinder{"test_material",
                                  render::vector{0.0, 0.0, 0.0}, 1.0,
                                  render::vector{0.0, 2.0, 0.0}};

  EXPECT_EQ(cylinder.get_material_name(), "test_material");
}

TEST(TestObjects, CylinderHitLateral) {
  render::Cylinder const cylinder{"mat", render::vector{0.0, 0.0, 0.0}, 1.0,
                                  render::vector{0.0, 2.0, 0.0}};

  render::ray const r{render::vector{5.0, 0.0, 0.0},
                      render::vector{-1.0, 0.0, 0.0}};

  render::HitRecord rec;
  render::Interval const ray_t{0.001, 1000.0};

  bool const hit = cylinder.hit(r, ray_t, rec);

  EXPECT_TRUE(hit);
  EXPECT_DOUBLE_EQ(rec.t, 4.0);

  EXPECT_NEAR(rec.point.x(), 1.0, 1e-10);
  EXPECT_NEAR(rec.point.y(), 0.0, 1e-10);
  EXPECT_NEAR(rec.point.z(), 0.0, 1e-10);

  EXPECT_NEAR(rec.normal.x(), 1.0, 1e-10);
  EXPECT_NEAR(rec.normal.y(), 0.0, 1e-10);
  EXPECT_NEAR(rec.normal.z(), 0.0, 1e-10);
}

TEST(TestObjects, CylinderLateralNormalIsUnitLengthForSmallRadius) {
  render::Cylinder const cylinder{"mat", render::vector{0.0, 0.0, 0.0}, 0.25,
                                  render::vector{0.0, 2.0, 0.0}};

  render::ray const r{render::vector{5.0, 0.0, 0.0},
                      render::vector{-1.0, 0.0, 0.0}};

  render::HitRecord rec;
  render::Interval const ray_t{0.001, 1000.0};

  ASSERT_TRUE(cylinder.hit(r, ray_t, rec));
  EXPECT_DOUBLE_EQ(rec.t, 4.75);
  EXPECT_NEAR(rec.normal.magnitude(), 1.0, 1e-10);
  EXPECT_NEAR(rec.normal.x(), 1.0, 1e-10);
}

TEST(TestObjects, CylinderHitTopCap) {
  render::Cylinder const cylinder{"mat", render::vector{0.0, 0.0, 0.0}, 1.0,
                                  render::vector{0.0, 2.0, 0.0}};

  render::ray const r{render::vector{0.0, 5.0, 0.0},
                      render::vector{0.0, -1.0, 0.0}};

  render::HitRecord rec;
  render::Interval const ray_t{0.001, 1000.0};

  bool const hit = cylinder.hit(r, ray_t, rec);

  EXPECT_TRUE(hit);
  EXPECT_DOUBLE_EQ(rec.t, 4.0);

  EXPECT_NEAR(rec.point.y(), 1.0, 1e-10);
  EXPECT_NEAR(rec.normal.y(), 1.0, 1e-10);
}

TEST(TestObjects, CylinderHitBottomCap) {
  render::Cylinder const cylinder{"mat", render::vector{0.0, 0.0, 0.0}, 1.0,
                                  render::vector{0.0, 2.0, 0.0}};

  render::ray const r{render::vector{0.0, -5.0, 0.0},
                      render::vector{0.0, 1.0, 0.0}};

  render::HitRecord rec;
  render::Interval const ray_t{0.001, 1000.0};

  bool const hit = cylinder.hit(r, ray_t, rec);

  EXPECT_TRUE(hit);
  EXPECT_DOUBLE_EQ(rec.t, 4.0);

  EXPECT_NEAR(rec.point.y(), -1.0, 1e-10);
  EXPECT_NEAR(rec.normal.y(), -1.0, 1e-10);
}

TEST(TestObjects, CylinderNoHit) {
  render::Cylinder const cylinder{"mat", render::vector{0.0, 0.0, 0.0}, 1.0,
                                  render::vector{0.0, 2.0, 0.0}};

  render::ray const r{render::vector{5.0, 0.0, 0.0},
                      render::vector{0.0, 1.0, 0.0}};

  render::HitRecord rec;
  render::Interval const ray_t{0.001, 1000.0};

  bool const hit = cylinder.hit(r, ray_t, rec);

  EXPECT_FALSE(hit);
}

TEST(TestObjects, CylinderParallelToAxis) {
  render::Cylinder const cylinder{"mat", render::vector{0.0, 0.0, 0.0}, 1.0,
                                  render::vector{0.0, 2.0, 0.0}};

  render::ray const r{render::vector{2.0, -5.0, 0.0},
                      render::vector{0.0, 1.0, 0.0}};

  render::HitRecord rec;
  render::Interval const ray_t{0.001, 1000.0};

  bool const hit = cylinder.hit(r, ray_t, rec);

  EXPECT_FALSE(hit);
}
