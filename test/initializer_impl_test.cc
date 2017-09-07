#include <config.h>

#include <cmath>
#include <vector>
#include <gtest/gtest.h>
#include <primitiv/cpu_device.h>
#include <primitiv/error.h>
#include <primitiv/initializer_impl.h>
#include <primitiv/shape.h>
#include <test_utils.h>

using std::vector;

namespace primitiv {
namespace initializers {

class InitializerImplTest : public testing::Test {
protected:
  CPUDevice dev;
};

TEST_F(InitializerImplTest, CheckConstant) {
  const Shape shape {3, 3, 3};
  for (float k : {1, 10, 100, 1000, 10000}) {
    const vector<float> expected(shape.size(), k);
    const Constant init(k);
    Tensor x = dev.new_tensor(shape);
    init.apply(x);
    EXPECT_EQ(expected, x.to_vector());
  }
}

TEST_F(InitializerImplTest, CheckUniform) {
  // NOTE(odashi): This test checks only range, mean and variance.
  struct TestCase {
    float lower, upper, mean, variance;
  };
  const vector<TestCase> test_cases {
    {-.1, .1, 0, .04/12},
    {0, 1, .5, 1./12},
    {-1, 0, -.5, 1./12},
    {-.70710678, .70710678, 0, 2./12},
  };
  const unsigned N = 768;

  for (const auto &tc : test_cases) {
    const Uniform init(tc.lower, tc.upper);
    Tensor x = dev.new_tensor({N, N});
    init.apply(x);
    double m1 = 0, m2 = 0;
    for (float v : x.to_vector()) {
      EXPECT_LT(tc.lower, v);
      EXPECT_GE(tc.upper, v);
      m1 += v;
      m2 += v * v;
    }
    const float mean = m1 / (N * N);
    const float variance = m2 / (N * N) - mean * mean;
    EXPECT_NEAR(tc.mean, mean, 1e-2);
    EXPECT_NEAR(tc.variance, variance, 1e-2);
  }
}

TEST_F(InitializerImplTest, CheckNormal) {
  // NOTE(odashi): This test checks only mean and SD.
  struct TestCase {
    float mean, sd;
  };
  const vector<TestCase> test_cases {
    {0, .1}, {0, 1}, {0, 3},
    {3, 2}, {-3, 2},
    {3, .5}, {-3, .5},
  };
  const unsigned N = 768;

  for (const auto &tc : test_cases) {
    const Normal init(tc.mean, tc.sd);
    Tensor x = dev.new_tensor({N, N});
    init.apply(x);
    double m1 = 0, m2 = 0;
    for (float v : x.to_vector()) {
      m1 += v;
      m2 += v * v;
    }
    const float mean = m1 / (N * N);
    const float sd = std::sqrt(m2 / (N * N) - mean * mean);
    EXPECT_NEAR(tc.mean, mean, 1e-2);
    EXPECT_NEAR(tc.sd, sd, 1e-2);
  }
}

TEST_F(InitializerImplTest, CheckXavierUniform) {
  const unsigned N = 768;
  const float scale = std::sqrt(6. / (N + N));
  const XavierUniform init;
  Tensor x = dev.new_tensor({N, N});
  init.apply(x);
  double m1 = 0, m2 = 0;
  for (float v : x.to_vector()) {
    EXPECT_LT(-scale, v);
    EXPECT_GE(scale, v);
    m1 += v;
    m2 += v * v;
  }
  const float mean = m1 / (N * N);
  const float variance = m2 / (N * N) - mean * mean;
  EXPECT_NEAR(0., mean, 1e-4);
  EXPECT_NEAR(2. / (N + N), variance, 1e-4);
}

TEST_F(InitializerImplTest, CheckInvalidXavierUniform) {
  const XavierUniform init;
  const std::vector<Shape> shapes {{2, 3, 4}, {2, 3, 4, 5}};
  for (const Shape &s : shapes) {
    Tensor x = dev.new_tensor(s);
    EXPECT_THROW(init.apply(x), Error);
  }
}

TEST_F(InitializerImplTest, CheckXavierNormal) {
  // NOTE(odashi): This test checks only mean and SD.
  const unsigned N = 768;
  const XavierNormal init;
  Tensor x = dev.new_tensor({N, N});
  init.apply(x);
  double m1 = 0, m2 = 0;
  for (float v : x.to_vector()) {
    m1 += v;
    m2 += v * v;
  }
  const float mean = m1 / (N * N);
  const float variance = m2 / (N * N) - mean * mean;
  EXPECT_NEAR(0., mean, 1e-4);
  EXPECT_NEAR(2. / (N + N), variance, 1e-4);
}

TEST_F(InitializerImplTest, CheckInvalidXavierNormal) {
  const XavierNormal init;
  const std::vector<Shape> shapes {{2, 3, 4}, {2, 3, 4, 5}};
  for (const Shape &s : shapes) {
    Tensor x = dev.new_tensor(s);
    EXPECT_THROW(init.apply(x), Error);
  }
}

}  // namespace initializers
}  // namespace primitiv
