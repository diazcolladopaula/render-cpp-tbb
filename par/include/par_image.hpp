#pragma once

#include <string>
#include <vector>

namespace render::par {

/**
 * @struct Pixel
 * @brief RGB pixel with 0-255 channels.
 */
struct Pixel {
  int r{0};
  int g{0};
  int b{0};
};

/**
 * @class Image
 * @brief Image stored as an array of structs (one Pixel per element).
 */
class Image {
public:
  Image(int width, int height);

  /**
   * @brief Fills the image from a flat buffer.
   * @param data Interleaved RGB values, row by row (3 * width * height).
   * @throws std::runtime_error If the buffer has the wrong size.
   */
  void set_data(std::vector<int> const &data);

  /**
   * @brief Writes the image as a plain-text PPM (P3) file.
   * @throws std::runtime_error If the file cannot be created.
   */
  void save_ppm(std::string const &filename) const;

private:
  int width_;
  int height_;
  std::vector<Pixel> pixels_;
};

} // namespace render::par
