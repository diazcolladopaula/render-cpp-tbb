#include "parsers.hpp"

#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <string>

#include <gtest/gtest.h>

class TempFile {
public:
  explicit TempFile(std::string const &content) {
    static int counter = 0;
    filename_ = "temp_test_file_" + std::to_string(counter++) + ".txt";

    std::ofstream file(filename_);
    file << content;
    file.close();
  }

  ~TempFile() { std::filesystem::remove(filename_); }

  TempFile(TempFile const &) = delete;
  TempFile(TempFile &&) = delete;
  TempFile &operator=(TempFile const &) = delete;
  TempFile &operator=(TempFile &&) = delete;

  [[nodiscard]] std::string const &filename() const { return filename_; }

private:
  std::string filename_;
};

TEST(TestParsers, ConfigParserBasicFile) {
  std::string const content = "aspect_ratio: 16 9\n"
                              "image_width: 800\n"
                              "gamma: 2.2\n";

  TempFile const temp(content);

  render::RenderConfig const config =
      render::ConfigParser::parse(temp.filename());

  EXPECT_EQ(config.aspect_ratio_w, 16);
  EXPECT_EQ(config.aspect_ratio_h, 9);
  EXPECT_EQ(config.image_width, 800);
  EXPECT_DOUBLE_EQ(config.gamma, 2.2);
}

TEST(TestParsers, ConfigParserVectorValues) {
  std::string const content = "camera_position: 0.0 5.0 10.0\n"
                              "camera_target: 0.0 0.0 0.0\n"
                              "camera_north: 0.0 1.0 0.0\n";

  TempFile const temp(content);

  render::RenderConfig const config =
      render::ConfigParser::parse(temp.filename());

  EXPECT_DOUBLE_EQ(config.camera_position.x(), 0.0);
  EXPECT_DOUBLE_EQ(config.camera_position.y(), 5.0);
  EXPECT_DOUBLE_EQ(config.camera_position.z(), 10.0);

  EXPECT_DOUBLE_EQ(config.camera_target.x(), 0.0);
  EXPECT_DOUBLE_EQ(config.camera_target.y(), 0.0);
  EXPECT_DOUBLE_EQ(config.camera_target.z(), 0.0);
}

TEST(TestParsers, ConfigParserComments) {
  std::string const content = "# This is a comment\n"
                              "image_width: 1920\n"
                              "# Another comment\n"
                              "gamma: 2.0\n";

  TempFile const temp(content);

  render::RenderConfig const config =
      render::ConfigParser::parse(temp.filename());

  EXPECT_EQ(config.image_width, 1'920);
  EXPECT_DOUBLE_EQ(config.gamma, 2.0);
}

TEST(TestParsers, ConfigParserEmptyLines) {
  std::string const content = "image_width: 640\n"
                              "\n"
                              "\n"
                              "gamma: 1.8\n";

  TempFile const temp(content);

  render::RenderConfig const config =
      render::ConfigParser::parse(temp.filename());

  EXPECT_EQ(config.image_width, 640);
  EXPECT_DOUBLE_EQ(config.gamma, 1.8);
}

TEST(TestParsers, ConfigParserAllParameters) {
  std::string const content = "aspect_ratio: 16 9\n"
                              "image_width: 800\n"
                              "gamma: 2.2\n"
                              "camera_position: 1.0 2.0 3.0\n"
                              "camera_target: 0.0 0.0 0.0\n"
                              "camera_north: 0.0 1.0 0.0\n"
                              "field_of_view: 90.0\n"
                              "samples_per_pixel: 100\n"
                              "max_depth: 50\n"
                              "material_rng_seed: 12345\n"
                              "ray_rng_seed: 67890\n"
                              "background_dark_color: 0.1 0.2 0.3\n"
                              "background_light_color: 0.8 0.9 1.0\n";

  TempFile const temp(content);

  render::RenderConfig const config =
      render::ConfigParser::parse(temp.filename());

  EXPECT_EQ(config.aspect_ratio_w, 16);
  EXPECT_EQ(config.aspect_ratio_h, 9);
  EXPECT_EQ(config.image_width, 800);
  EXPECT_DOUBLE_EQ(config.gamma, 2.2);
  EXPECT_EQ(config.samples_per_pixel, 100);
  EXPECT_EQ(config.max_depth, 50);
  EXPECT_EQ(config.material_rng_seed, 12'345);
  EXPECT_EQ(config.ray_rng_seed, 67'890);
  EXPECT_DOUBLE_EQ(config.field_of_view, 90.0);
}

TEST(TestParsers, ConfigParserInvalidFile) {
  EXPECT_THROW(
      {
        auto const config = render::ConfigParser::parse("nonexistent_file.txt");
        (void)config;
      },
      std::runtime_error);
}

TEST(TestParsers, ConfigParserInvalidKey) {
  std::string const content = "invalid_key: 100\n";
  TempFile const temp(content);

  EXPECT_THROW(
      {
        auto const config = render::ConfigParser::parse(temp.filename());
        (void)config;
      },
      std::runtime_error);
}

TEST(TestParsers, ConfigParserInvalidIntValue) {
  std::string const content = "image_width: not_a_number\n";
  TempFile const temp(content);

  EXPECT_THROW(
      {
        auto const config = render::ConfigParser::parse(temp.filename());
        (void)config;
      },
      std::runtime_error);
}

TEST(TestParsers, ConfigParserInvalidVectorValue) {
  std::string const content = "camera_position: 1.0 2.0\n";
  TempFile const temp(content);

  EXPECT_THROW(
      {
        auto const config = render::ConfigParser::parse(temp.filename());
        (void)config;
      },
      std::runtime_error);
}

TEST(TestParsers, ConfigParserExtraData) {
  std::string const content = "image_width: 800 extra_data\n";
  TempFile const temp(content);

  EXPECT_THROW(
      {
        auto const config = render::ConfigParser::parse(temp.filename());
        (void)config;
      },
      std::runtime_error);
}

TEST(TestParsers, ConfigParserOutOfRangeInt) {
  std::string const content = "image_width: 0\n";
  TempFile const temp(content);

  EXPECT_THROW(
      {
        auto const config = render::ConfigParser::parse(temp.filename());
        (void)config;
      },
      std::runtime_error);
}

TEST(TestParsers, ConfigParserOutOfRangeDouble) {
  std::string const content = "field_of_view: 200.0\n";
  TempFile const temp(content);

  EXPECT_THROW(
      {
        auto const config = render::ConfigParser::parse(temp.filename());
        (void)config;
      },
      std::runtime_error);
}

TEST(TestParsers, SceneParserMatteMaterial) {
  std::string const content = "matte: red 0.8 0.2 0.2\n";
  TempFile const temp(content);

  render::SceneData const scene = render::SceneParser::parse(temp.filename());

  EXPECT_EQ(scene.materials.size(), 1);
  EXPECT_EQ(scene.materials[0]->get_name(), "red");
}

TEST(TestParsers, SceneParserMetalMaterial) {
  std::string const content = "metal: silver 0.9 0.9 0.9 0.1\n";
  TempFile const temp(content);

  render::SceneData const scene = render::SceneParser::parse(temp.filename());

  EXPECT_EQ(scene.materials.size(), 1);
  EXPECT_EQ(scene.materials[0]->get_name(), "silver");
}

TEST(TestParsers, SceneParserRefractiveMaterial) {
  std::string const content = "refractive: glass 1.5\n";
  TempFile const temp(content);

  render::SceneData const scene = render::SceneParser::parse(temp.filename());

  EXPECT_EQ(scene.materials.size(), 1);
  EXPECT_EQ(scene.materials[0]->get_name(), "glass");
}

TEST(TestParsers, SceneParserSphere) {
  std::string const content = "matte: red 1.0 0.0 0.0\n"
                              "sphere: 0.0 0.0 0.0 1.0 red\n";
  TempFile const temp(content);

  render::SceneData const scene = render::SceneParser::parse(temp.filename());

  EXPECT_EQ(scene.materials.size(), 1);
  EXPECT_EQ(scene.objects.size(), 1);
  EXPECT_EQ(scene.objects[0]->get_material_name(), "red");
}

TEST(TestParsers, SceneParserCylinder) {
  std::string const content = "metal: silver 0.9 0.9 0.9 0.0\n"
                              "cylinder: 0.0 0.0 0.0 1.0 0.0 2.0 0.0 silver\n";
  TempFile const temp(content);

  render::SceneData const scene = render::SceneParser::parse(temp.filename());

  EXPECT_EQ(scene.materials.size(), 1);
  EXPECT_EQ(scene.objects.size(), 1);
  EXPECT_EQ(scene.objects[0]->get_material_name(), "silver");
}

TEST(TestParsers, SceneParserMultipleMaterials) {
  std::string const content = "matte: red 1.0 0.0 0.0\n"
                              "metal: silver 0.9 0.9 0.9 0.1\n"
                              "refractive: glass 1.5\n";
  TempFile const temp(content);

  render::SceneData const scene = render::SceneParser::parse(temp.filename());

  EXPECT_EQ(scene.materials.size(), 3);
}

TEST(TestParsers, SceneParserMultipleObjects) {
  std::string const content = "matte: red 1.0 0.0 0.0\n"
                              "sphere: 0.0 0.0 0.0 1.0 red\n"
                              "sphere: 2.0 0.0 0.0 0.5 red\n"
                              "cylinder: 0.0 -2.0 0.0 0.5 0.0 2.0 0.0 red\n";
  TempFile const temp(content);

  render::SceneData const scene = render::SceneParser::parse(temp.filename());

  EXPECT_EQ(scene.materials.size(), 1);
  EXPECT_EQ(scene.objects.size(), 3);
}

TEST(TestParsers, SceneParserComments) {
  std::string const content = "# Comment\n"
                              "matte: red 1.0 0.0 0.0\n"
                              "# Another comment\n"
                              "sphere: 0.0 0.0 0.0 1.0 red\n";
  TempFile const temp(content);

  render::SceneData const scene = render::SceneParser::parse(temp.filename());

  EXPECT_EQ(scene.materials.size(), 1);
  EXPECT_EQ(scene.objects.size(), 1);
}

TEST(TestParsers, SceneParserEmptyLines) {
  std::string const content = "matte: red 1.0 0.0 0.0\n"
                              "\n"
                              "\n"
                              "sphere: 0.0 0.0 0.0 1.0 red\n";
  TempFile const temp(content);

  render::SceneData const scene = render::SceneParser::parse(temp.filename());

  EXPECT_EQ(scene.materials.size(), 1);
  EXPECT_EQ(scene.objects.size(), 1);
}

TEST(TestParsers, SceneParserInvalidFile) {
  EXPECT_THROW(
      {
        auto const scene = render::SceneParser::parse("nonexistent_scene.txt");
        (void)scene;
      },
      std::runtime_error);
}

TEST(TestParsers, SceneParserDuplicateMaterialName) {
  std::string const content = "matte: red 1.0 0.0 0.0\n"
                              "metal: red 0.9 0.9 0.9 0.1\n";
  TempFile const temp(content);

  EXPECT_THROW(
      {
        auto const scene = render::SceneParser::parse(temp.filename());
        (void)scene;
      },
      std::runtime_error);
}

TEST(TestParsers, SceneParserUndefinedMaterial) {
  std::string const content = "matte: red 1.0 0.0 0.0\n"
                              "sphere: 0.0 0.0 0.0 1.0 blue\n";
  TempFile const temp(content);

  EXPECT_THROW(
      {
        auto const scene = render::SceneParser::parse(temp.filename());
        (void)scene;
      },
      std::runtime_error);
}

TEST(TestParsers, SceneParserInvalidSphereRadius) {
  std::string const content = "matte: red 1.0 0.0 0.0\n"
                              "sphere: 0.0 0.0 0.0 -1.0 red\n";
  TempFile const temp(content);

  EXPECT_THROW(
      {
        auto const scene = render::SceneParser::parse(temp.filename());
        (void)scene;
      },
      std::runtime_error);
}

TEST(TestParsers, SceneParserInvalidCylinderRadius) {
  std::string const content = "matte: red 1.0 0.0 0.0\n"
                              "cylinder: 0.0 0.0 0.0 0.0 0.0 2.0 0.0 red\n";
  TempFile const temp(content);

  EXPECT_THROW(
      {
        auto const scene = render::SceneParser::parse(temp.filename());
        (void)scene;
      },
      std::runtime_error);
}

TEST(TestParsers, SceneParserInvalidEntityType) {
  std::string const content = "unknown_entity: param1 param2\n";
  TempFile const temp(content);

  EXPECT_THROW(
      {
        auto const scene = render::SceneParser::parse(temp.filename());
        (void)scene;
      },
      std::runtime_error);
}

TEST(TestParsers, SceneParserInsufficientParameters) {
  std::string const content = "matte: red 1.0 0.0\n";
  TempFile const temp(content);

  EXPECT_THROW(
      {
        auto const scene = render::SceneParser::parse(temp.filename());
        (void)scene;
      },
      std::runtime_error);
}

TEST(TestParsers, SceneParserExcessiveParameters) {
  std::string const content = "matte: red 1.0 0.0 0.0 0.5\n";
  TempFile const temp(content);

  EXPECT_THROW(
      {
        auto const scene = render::SceneParser::parse(temp.filename());
        (void)scene;
      },
      std::runtime_error);
}

TEST(TestParsers, SceneParserCompleteScene) {
  std::string const content = "# Materiales\n"
                              "matte: ground 0.5 0.5 0.5\n"
                              "metal: silver 0.9 0.9 0.9 0.0\n"
                              "refractive: glass 1.5\n"
                              "\n"
                              "# Objetos\n"
                              "sphere: 0.0 -100.5 0.0 100.0 ground\n"
                              "sphere: 0.0 0.0 0.0 0.5 glass\n"
                              "sphere: 1.0 0.0 0.0 0.5 silver\n"
                              "cylinder: -1.0 0.0 0.0 0.3 0.0 1.0 0.0 silver\n";
  TempFile const temp(content);

  render::SceneData const scene = render::SceneParser::parse(temp.filename());

  EXPECT_EQ(scene.materials.size(), 3);
  EXPECT_EQ(scene.objects.size(), 4);
}
