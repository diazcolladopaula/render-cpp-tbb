#include "ray.hpp"

#include <sstream>

#include <gtest/gtest.h>

#include "vector.hpp"

TEST(TestRay, ConstructorAndGetters) {
  using render::ray;
  using render::vector;

  vector const origin{1.0, 2.0, 3.0};
  vector const direction{4.0, 5.0, 6.0};
  ray const r{origin, direction};

  EXPECT_DOUBLE_EQ(r.origin().x(), 1.0);
  EXPECT_DOUBLE_EQ(r.origin().y(), 2.0);
  EXPECT_DOUBLE_EQ(r.origin().z(), 3.0);

  EXPECT_DOUBLE_EQ(r.direction().x(), 4.0);
  EXPECT_DOUBLE_EQ(r.direction().y(), 5.0);
  EXPECT_DOUBLE_EQ(r.direction().z(), 6.0);
}

TEST(TestRay, AtLambdaZero) {
  render::ray const r{render::vector{1.0, 2.0, 3.0},
                      render::vector{4.0, 5.0, 6.0}};

  render::vector const point = r.at(0.0);

  EXPECT_DOUBLE_EQ(point.x(), 1.0);
  EXPECT_DOUBLE_EQ(point.y(), 2.0);
  EXPECT_DOUBLE_EQ(point.z(), 3.0);
}

TEST(TestRay, AtLambdaOne) {
  render::ray const r{render::vector{1.0, 2.0, 3.0},
                      render::vector{4.0, 5.0, 6.0}};

  render::vector const point = r.at(1.0);

  EXPECT_DOUBLE_EQ(point.x(), 5.0);
  EXPECT_DOUBLE_EQ(point.y(), 7.0);
  EXPECT_DOUBLE_EQ(point.z(), 9.0);
}

TEST(TestRay, AtLambdaArbitrary) {
  render::ray const r{render::vector{0.0, 0.0, 0.0},
                      render::vector{1.0, 2.0, 3.0}};

  render::vector const point = r.at(2.5);

  EXPECT_DOUBLE_EQ(point.x(), 2.5);
  EXPECT_DOUBLE_EQ(point.y(), 5.0);
  EXPECT_DOUBLE_EQ(point.z(), 7.5);
}

TEST(TestRay, AtLambdaNegative) {
  render::ray const r{render::vector{5.0, 5.0, 5.0},
                      render::vector{1.0, 1.0, 1.0}};

  render::vector const point = r.at(-2.0);

  EXPECT_DOUBLE_EQ(point.x(), 3.0);
  EXPECT_DOUBLE_EQ(point.y(), 3.0);
  EXPECT_DOUBLE_EQ(point.z(), 3.0);
}

TEST(TestRay, OperatorOstream) {
  render::ray const r{render::vector{1.5, 2.0, -3.25},
                      render::vector{0.5, 1.0, 2.0}};
  std::stringstream ss;
  ss << r;

  EXPECT_EQ(ss.str(),
            "ray(origin: vector(1.5, 2, -3.25), direction: vector(0.5, 1, 2))");
}
