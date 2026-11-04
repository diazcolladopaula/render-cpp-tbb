#include "par_image.hpp"
#include <cstdio>
#include <fstream>
#include <stdexcept>
#include <string>
#include <vector>

#include <gtest/gtest.h>

TEST(ParImageTest, SetDataThrowsOnInvalidSize) {
  using render::par::Image;

  Image img(10, 10);
  std::vector<int> const bad_data(299, 0);

  EXPECT_THROW(img.set_data(bad_data), std::runtime_error);
}

TEST(ParImageTest, SetDataSucceedsWithCorrectSize) {
  using render::par::Image;

  Image img(10, 10);
  std::vector<int> const good_data(300, 128);

  EXPECT_NO_THROW(img.set_data(good_data));
}

TEST(ParImageTest, SavePPMWritesCorrectHeaderAndData) {
  using render::par::Image;

  std::string const filename = "test_output_par.ppm";
  Image img(2, 1);

  std::vector<int> const data = {255, 0, 0, 0, 255, 0};
  img.set_data(data);
  img.save_ppm(filename);

  std::ifstream file(filename);
  ASSERT_TRUE(file.is_open()) << "Could not open file: " << filename;

  std::string line;

  std::getline(file, line);
  EXPECT_EQ(line, "P3");

  std::getline(file, line);
  EXPECT_EQ(line, "2 1");

  std::getline(file, line);
  EXPECT_EQ(line, "255");

  std::getline(file, line);
  EXPECT_EQ(line, "255 0 0");

  std::getline(file, line);
  EXPECT_EQ(line, "0 255 0");

  file.close();
  static_cast<void>(std::remove(filename.c_str()));
}

TEST(ParImageTest, SavePPMThrowsOnInvalidPath) {
  using render::par::Image;

  Image img(2, 2);
  std::vector<int> const data(12, 0);
  img.set_data(data);

  std::string const invalid_path = "/invalid/path/that/does/not/exist/test.ppm";

  EXPECT_THROW(img.save_ppm(invalid_path), std::runtime_error);
}

TEST(ParImageTest, ConstructorInitializesCorrectly) {
  using render::par::Image;

  EXPECT_NO_THROW({ Image const img(100, 50); });

  EXPECT_NO_THROW({ Image const img(1, 1); });
}

TEST(ParImageTest, SetDataWorksWithLargeImage) {
  using render::par::Image;

  Image img(100, 100);
  std::vector<int> const large_data(static_cast<size_t>(100 * 100 * 3), 128);

  EXPECT_NO_THROW(img.set_data(large_data));
}

TEST(ParImageTest, SavePPMWritesCorrectRGBValues) {
  using render::par::Image;

  std::string const filename = "test_rgb_par.ppm";
  Image img(3, 1);

  // Red, green, blue
  std::vector<int> const data = {
      255, 0,   0,  // Pixel 1: Rojo
      0,   255, 0,  // Pixel 2: Verde
      0,   0,   255 // Pixel 3: Azul
  };

  img.set_data(data);
  img.save_ppm(filename);

  std::ifstream file(filename);
  ASSERT_TRUE(file.is_open());

  std::string line;

  // Skip the 3-line header
  std::getline(file, line); // P3
  std::getline(file, line); // 3 1
  std::getline(file, line); // 255

  // Check the pixels
  std::getline(file, line);
  EXPECT_EQ(line, "255 0 0");

  std::getline(file, line);
  EXPECT_EQ(line, "0 255 0");

  std::getline(file, line);
  EXPECT_EQ(line, "0 0 255");

  file.close();
  static_cast<void>(std::remove(filename.c_str()));
}

TEST(ParImageTest, SetDataExactSize) {
  using render::par::Image;

  Image img(5, 4);
  std::vector<int> const exact_data(static_cast<size_t>(5 * 4 * 3), 100);

  EXPECT_NO_THROW(img.set_data(exact_data));
}

TEST(ParImageTest, SetDataTooSmall) {
  using render::par::Image;

  Image img(10, 10);
  std::vector<int> const small_data(100, 0);

  EXPECT_THROW(img.set_data(small_data), std::runtime_error);
}

TEST(ParImageTest, SetDataTooLarge) {
  using render::par::Image;

  Image img(10, 10);
  std::vector<int> const large_data(500, 0);

  EXPECT_THROW(img.set_data(large_data), std::runtime_error);
}
