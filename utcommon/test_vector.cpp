#include "vector.hpp"

#include <sstream>

#include <gtest/gtest.h>

TEST(TestVector, ConstructorAndGetters) {
  using render::vector;
  vector const v{1.1, -2.2, 3.3};

  EXPECT_DOUBLE_EQ(v.x(), 1.1);
  EXPECT_DOUBLE_EQ(v.y(), -2.2);
  EXPECT_DOUBLE_EQ(v.z(), 3.3);
}

TEST(TestVector, MagnitudeZero) {
  render::vector const vec{0.0, 0.0, 0.0};
  EXPECT_DOUBLE_EQ(vec.magnitude(), 0.0);
}

TEST(TestVector, MagnitudePositive) {
  render::vector const vec{3.0, 4.0, 0.0};
  EXPECT_DOUBLE_EQ(vec.magnitude(), 5.0);
}

TEST(TestVector, MagnitudeSquared) {
  render::vector const vec{3.0, 4.0, 0.0};
  EXPECT_DOUBLE_EQ(vec.magnitude_squared(), 25.0);
}

TEST(TestVector, DotProduct) {
  render::vector const v1{1.0, 2.0, 3.0};
  render::vector const v2{4.0, -5.0, 6.0};
  EXPECT_DOUBLE_EQ(v1.dot(v2), 12.0);
}

TEST(TestVector, DotProductOrthogonal) {
  render::vector const v1{1.0, 0.0, 0.0};
  render::vector const v2{0.0, 1.0, 0.0};
  EXPECT_DOUBLE_EQ(v1.dot(v2), 0.0);
}

TEST(TestVector, CrossProductBasis) {
  render::vector const i{1.0, 0.0, 0.0};
  render::vector const j{0.0, 1.0, 0.0};
  render::vector const k{0.0, 0.0, 1.0};

  render::vector const result = i.cross(j);
  EXPECT_DOUBLE_EQ(result.x(), k.x());
  EXPECT_DOUBLE_EQ(result.y(), k.y());
  EXPECT_DOUBLE_EQ(result.z(), k.z());
}

TEST(TestVector, CrossProductParallel) {
  render::vector const v1{1.0, 2.0, 3.0};
  render::vector const v2{2.0, 4.0, 6.0};

  render::vector const result = v1.cross(v2);
  EXPECT_DOUBLE_EQ(result.x(), 0.0);
  EXPECT_DOUBLE_EQ(result.y(), 0.0);
  EXPECT_DOUBLE_EQ(result.z(), 0.0);
}

TEST(TestVector, Normalize) {
  render::vector const v{3.0, 4.0, 0.0};
  render::vector const norm_v = v.normalize();

  EXPECT_DOUBLE_EQ(norm_v.x(), 0.6);
  EXPECT_DOUBLE_EQ(norm_v.y(), 0.8);
  EXPECT_DOUBLE_EQ(norm_v.z(), 0.0);
  EXPECT_DOUBLE_EQ(norm_v.magnitude(), 1.0);
}

TEST(TestVector, NormalizeZero) {
  render::vector const v{0.0, 0.0, 0.0};
  render::vector const norm_v = v.normalize();
  EXPECT_DOUBLE_EQ(norm_v.x(), 0.0);
  EXPECT_DOUBLE_EQ(norm_v.y(), 0.0);
  EXPECT_DOUBLE_EQ(norm_v.z(), 0.0);
}

TEST(TestVector, OperatorUnaryMinus) {
  render::vector const v{1.0, -2.0, 3.0};
  render::vector const result = -v;
  EXPECT_DOUBLE_EQ(result.x(), -1.0);
  EXPECT_DOUBLE_EQ(result.y(), 2.0);
  EXPECT_DOUBLE_EQ(result.z(), -3.0);
}

TEST(TestVector, OperatorPlusEquals) {
  render::vector v1{1.0, 2.0, 3.0};
  render::vector const v2{4.0, 5.0, 6.0};
  v1 += v2;

  EXPECT_DOUBLE_EQ(v1.x(), 5.0);
  EXPECT_DOUBLE_EQ(v1.y(), 7.0);
  EXPECT_DOUBLE_EQ(v1.z(), 9.0);
}

TEST(TestVector, OperatorMinusEquals) {
  render::vector v1{10.0, 8.0, 6.0};
  render::vector const v2{1.0, 2.0, 3.0};
  v1 -= v2;

  EXPECT_DOUBLE_EQ(v1.x(), 9.0);
  EXPECT_DOUBLE_EQ(v1.y(), 6.0);
  EXPECT_DOUBLE_EQ(v1.z(), 3.0);
}

TEST(TestVector, OperatorMultiplyEqualsScalar) {
  render::vector v{1.0, -2.0, 3.0};
  v *= 3.0;

  EXPECT_DOUBLE_EQ(v.x(), 3.0);
  EXPECT_DOUBLE_EQ(v.y(), -6.0);
  EXPECT_DOUBLE_EQ(v.z(), 9.0);
}

TEST(TestVector, OperatorDivideEqualsScalar) {
  render::vector v{3.0, -6.0, 9.0};
  v /= 3.0;

  EXPECT_DOUBLE_EQ(v.x(), 1.0);
  EXPECT_DOUBLE_EQ(v.y(), -2.0);
  EXPECT_DOUBLE_EQ(v.z(), 3.0);
}

TEST(TestVector, OperatorPlusFree) {
  render::vector const v1{1.0, 2.0, 3.0};
  render::vector const v2{4.0, 5.0, 6.0};
  render::vector const result = v1 + v2;

  EXPECT_DOUBLE_EQ(result.x(), 5.0);
  EXPECT_DOUBLE_EQ(result.y(), 7.0);
  EXPECT_DOUBLE_EQ(result.z(), 9.0);
}

TEST(TestVector, OperatorMinusFree) {
  render::vector const v1{10.0, 8.0, 6.0};
  render::vector const v2{1.0, 2.0, 3.0};
  render::vector const result = v1 - v2;

  EXPECT_DOUBLE_EQ(result.x(), 9.0);
  EXPECT_DOUBLE_EQ(result.y(), 6.0);
  EXPECT_DOUBLE_EQ(result.z(), 3.0);
}

TEST(TestVector, OperatorMultiplyFreeComponentwise) {
  render::vector const v1{1.0, 2.0, 3.0};
  render::vector const v2{4.0, 5.0, 6.0};
  render::vector const result = v1 * v2;

  EXPECT_DOUBLE_EQ(result.x(), 4.0);
  EXPECT_DOUBLE_EQ(result.y(), 10.0);
  EXPECT_DOUBLE_EQ(result.z(), 18.0);
}

TEST(TestVector, OperatorMultiplyFreeScalarRight) {
  render::vector const v{1.0, 2.0, 3.0};
  render::vector const result = v * 3.0;

  EXPECT_DOUBLE_EQ(result.x(), 3.0);
  EXPECT_DOUBLE_EQ(result.y(), 6.0);
  EXPECT_DOUBLE_EQ(result.z(), 9.0);
}

TEST(TestVector, OperatorMultiplyFreeScalarLeft) {
  render::vector const v{1.0, 2.0, 3.0};
  render::vector const result = 3.0 * v;

  EXPECT_DOUBLE_EQ(result.x(), 3.0);
  EXPECT_DOUBLE_EQ(result.y(), 6.0);
  EXPECT_DOUBLE_EQ(result.z(), 9.0);
}

TEST(TestVector, OperatorDivideFreeScalar) {
  render::vector const v{3.0, 6.0, 9.0};
  render::vector const result = v / 3.0;

  EXPECT_DOUBLE_EQ(result.x(), 1.0);
  EXPECT_DOUBLE_EQ(result.y(), 2.0);
  EXPECT_DOUBLE_EQ(result.z(), 3.0);
}

TEST(TestVector, OperatorOstream) {
  render::vector const v{1.5, 2.0, -3.25};
  std::stringstream ss;
  ss << v;

  EXPECT_EQ(ss.str(), "vector(1.5, 2, -3.25)");
}
