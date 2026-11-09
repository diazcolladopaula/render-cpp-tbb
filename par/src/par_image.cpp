#include "par_image.hpp"
#include <cstddef>
#include <fstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace render::par {

Image::Image(int width, int height)
    : width_{width}, height_{height},
      pixels_(static_cast<size_t>(width * height)) {}

void Image::set_data(std::vector<int> const &data) {
  if (data.size() !=
      static_cast<size_t>(static_cast<long long>(width_) * height_ * 3)) {
    throw std::runtime_error("Image data has the wrong size");
  }

  size_t idx = 0;
  for (auto &pixel : pixels_) {
    pixel.r = data[idx++];
    pixel.g = data[idx++];
    pixel.b = data[idx++];
  }
}

void Image::save_ppm(std::string const &filename) const {
  std::ofstream file(filename);
  if (!file.is_open()) {
    throw std::runtime_error("Could not create file: " + filename);
  }

  file << "P3\n";
  file << width_ << " " << height_ << "\n";
  file << "255\n";

  for (auto const &pixel : pixels_) {
    file << pixel.r << " " << pixel.g << " " << pixel.b << "\n";
  }
}

} // namespace render::par
