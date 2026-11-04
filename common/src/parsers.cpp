#include "parsers.hpp"
#include "material.hpp"
#include "objects.hpp"
#include "vector.hpp"
#include <exception>
#include <fstream>
#include <limits>
#include <map>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>

namespace {

void throw_invalid_value(std::string const &key, std::string const &line) {
  throw std::runtime_error("Invalid value for key: [" + key + "]\n Content: \"" +
                           line + "\"");
}

// Groups the state shared by the per-line parsing helpers.
struct ParseContext {
  std::stringstream *ss_ptr;             // Remaining tokens of the line
  std::string const *key_ptr;            // Key or entity type of the line
  std::string const *line_for_error_ptr; // Original line, for error messages
};

// Throws if there are unexpected tokens left on the line.
void check_extra_data(ParseContext &ctx) {
  std::string extra_token;
  if (*(ctx.ss_ptr) >> extra_token) {
    std::string rest_of_line;
    std::getline(*(ctx.ss_ptr), rest_of_line);
    std::string extra = extra_token + rest_of_line;
    extra.erase(0, extra.find_first_not_of(" \t"));

    throw std::runtime_error("Unexpected extra data after the value for key: [" +
                             *(ctx.key_ptr) + "]\n Extra: \"" + extra + "\"");
  }
}

double parse_double(ParseContext &ctx, double min_val, double max_val) {
  double val{0.0};
  if (not(*(ctx.ss_ptr) >> val) or val < min_val or val > max_val) {
    throw_invalid_value(*(ctx.key_ptr), *(ctx.line_for_error_ptr));
  }
  check_extra_data(ctx);
  return val;
}

int parse_int(ParseContext &ctx, int min_val, int max_val) {
  int val{0};
  if (not(*(ctx.ss_ptr) >> val) or val < min_val or val > max_val) {
    throw_invalid_value(*(ctx.key_ptr), *(ctx.line_for_error_ptr));
  }
  check_extra_data(ctx);
  return val;
}

render::vector parse_vector(ParseContext &ctx, double min_val, double max_val) {
  double x{0.0}, y{0.0}, z{0.0};
  if (not(*(ctx.ss_ptr) >> x >> y >> z)) {
    throw_invalid_value(*(ctx.key_ptr), *(ctx.line_for_error_ptr));
  }
  if (x < min_val or x > max_val or y < min_val or y > max_val or z < min_val or
      z > max_val) {
    throw_invalid_value(*(ctx.key_ptr), *(ctx.line_for_error_ptr));
  }
  check_extra_data(ctx);
  return {x, y, z};
}

void parse_aspect_ratio(ParseContext &ctx, int &w, int &h) {
  if (not(*(ctx.ss_ptr) >> w >> h) or w <= 0 or h <= 0) {
    throw_invalid_value(*(ctx.key_ptr), *(ctx.line_for_error_ptr));
  }
  check_extra_data(ctx);
}

void dispatch_key_parser(render::RenderConfig &config, ParseContext &ctx) {
  auto const inf_pos = std::numeric_limits<double>::infinity();
  auto const inf_neg = -std::numeric_limits<double>::infinity();
  auto const int_max = std::numeric_limits<int>::max();

  if (*(ctx.key_ptr) == "aspect_ratio:") {
    parse_aspect_ratio(ctx, config.aspect_ratio_w, config.aspect_ratio_h);
  } else if (*(ctx.key_ptr) == "image_width:") {
    config.image_width = parse_int(ctx, 1, int_max);
  } else if (*(ctx.key_ptr) == "gamma:") {
    config.gamma = parse_double(ctx, 0.001, inf_pos);
  } else if (*(ctx.key_ptr) == "camera_position:") {
    config.camera_position = parse_vector(ctx, inf_neg, inf_pos);
  } else if (*(ctx.key_ptr) == "camera_target:") {
    config.camera_target = parse_vector(ctx, inf_neg, inf_pos);
  } else if (*(ctx.key_ptr) == "camera_north:") {
    config.camera_north = parse_vector(ctx, inf_neg, inf_pos);
  } else if (*(ctx.key_ptr) == "field_of_view:") {
    config.field_of_view = parse_double(ctx, 0.0001, 179.9999);
  } else if (*(ctx.key_ptr) == "samples_per_pixel:") {
    config.samples_per_pixel = parse_int(ctx, 1, int_max);
  } else if (*(ctx.key_ptr) == "max_depth:") {
    config.max_depth = parse_int(ctx, 1, int_max);
  } else if (*(ctx.key_ptr) == "material_rng_seed:") {
    config.material_rng_seed = parse_int(ctx, 1, int_max);
  } else if (*(ctx.key_ptr) == "ray_rng_seed:") {
    config.ray_rng_seed = parse_int(ctx, 1, int_max);
  } else if (*(ctx.key_ptr) == "background_dark_color:") {
    config.background_dark_color = parse_vector(ctx, 0.0, 1.0);
  } else if (*(ctx.key_ptr) == "background_light_color:") {
    config.background_light_color = parse_vector(ctx, 0.0, 1.0);
  } else {
    throw std::runtime_error("Unknown configuration key: [" +
                             *(ctx.key_ptr) + "]");
  }
}

void process_line(render::RenderConfig &config, std::string const &line,
                  int line_number) {
  std::stringstream ss_line(line);
  std::string key;
  if (not std::getline(ss_line, key, ':')) {
    throw std::runtime_error(
        "Unrecognized or malformed key (expected e.g. 'image_width:')");
  }
  key += ":";
  std::string value_part;
  std::getline(ss_line, value_part);
  std::stringstream ss_value(value_part);
  std::string const full_line_for_error = key + value_part;
  ParseContext ctx{&ss_value, &key, &full_line_for_error};

  try {
    dispatch_key_parser(config, ctx);
  } catch (std::exception const &e) {
    throw std::runtime_error("Line " + std::to_string(line_number) +
                             ":\n" + e.what());
  }
}

void parse_matte(
    ParseContext &ctx, render::SceneData &scene_data,
    std::map<std::string, render::material::Material *> &material_map) {
  std::string name;
  double r{0.0}, g{0.0}, b{0.0};
  if (not(*(ctx.ss_ptr) >> name >> r >> g >> b)) {
    throw std::runtime_error("Invalid parameters for matte material");
  }
  if (material_map.contains(name)) {
    throw std::runtime_error("Material [" + name + "] is already defined");
  }
  check_extra_data(ctx);
  auto mat =
      std::make_unique<render::material::Matte>(name, render::vector{r, g, b});
  material_map[name] = mat.get();
  scene_data.materials.push_back(std::move(mat));
}

void parse_metal(
    ParseContext &ctx, render::SceneData &scene_data,
    std::map<std::string, render::material::Material *> &material_map) {
  std::string name;
  double r{0.0}, g{0.0}, b{0.0}, fuzz{0.0};
  if (not(*(ctx.ss_ptr) >> name >> r >> g >> b >> fuzz)) {
    throw std::runtime_error(
        "Invalid parameters for metal material");
  }
  if (material_map.contains(name)) {
    throw std::runtime_error("Material [" + name + "] is already defined");
  }
  check_extra_data(ctx);
  auto mat = std::make_unique<render::material::Metal>(
      name, render::vector{r, g, b}, fuzz);
  material_map[name] = mat.get();
  scene_data.materials.push_back(std::move(mat));
}

void parse_refractive(
    ParseContext &ctx, render::SceneData &scene_data,
    std::map<std::string, render::material::Material *> &material_map) {
  std::string name;
  double ior{0.0};
  if (not(*(ctx.ss_ptr) >> name >> ior)) {
    throw std::runtime_error(
        "Invalid parameters for refractive material");
  }
  if (material_map.contains(name)) {
    throw std::runtime_error("Material [" + name + "] is already defined");
  }
  check_extra_data(ctx);
  auto mat = std::make_unique<render::material::Refractive>(name, ior);
  material_map[name] = mat.get();
  scene_data.materials.push_back(std::move(mat));
}

void parse_sphere(
    ParseContext &ctx, render::SceneData &scene_data,
    std::map<std::string, render::material::Material *> const &material_map) {
  double cx{0.0}, cy{0.0}, cz{0.0}, radius{0.0};
  std::string mat_name;
  if (not(*(ctx.ss_ptr) >> cx >> cy >> cz >> radius >> mat_name)) {
    throw std::runtime_error("Invalid sphere parameters");
  }
  if (radius <= 0.0) {
    throw std::runtime_error("Invalid sphere parameters");
  }
  if (not material_map.contains(mat_name)) {
    throw std::runtime_error("Material not found: [" + mat_name + "]");
  }
  check_extra_data(ctx);
  scene_data.objects.push_back(std::make_unique<render::Sphere>(
      mat_name, render::vector{cx, cy, cz}, radius));
}

void parse_cylinder(
    ParseContext &ctx, render::SceneData &scene_data,
    std::map<std::string, render::material::Material *> const &material_map) {
  double cx{0.0}, cy{0.0}, cz{0.0}, radius{0.0};
  double ax{0.0}, ay{0.0}, az{0.0};
  std::string mat_name;
  if (not(*(ctx.ss_ptr) >> cx >> cy >> cz >> radius >> ax >> ay >> az >>
          mat_name)) {
    throw std::runtime_error("Invalid cylinder parameters");
  }
  if (radius <= 0.0) {
    throw std::runtime_error("Invalid cylinder parameters");
  }
  if (not material_map.contains(mat_name)) {
    throw std::runtime_error("Material not found: [" + mat_name + "]");
  }
  check_extra_data(ctx);
  scene_data.objects.push_back(
      std::make_unique<render::Cylinder>(mat_name, render::vector{cx, cy, cz},
                                         radius, render::vector{ax, ay, az}));
}

// Routes a scene line to the parser of its entity type.
void dispatch_scene_entity_parser(
    ParseContext &ctx, render::SceneData &scene_data,
    std::map<std::string, render::material::Material *> &material_map) {
  std::string const &type = *(ctx.key_ptr); // e.g. "matte:"

  if (type == "matte:") {
    parse_matte(ctx, scene_data, material_map);
  } else if (type == "metal:") {
    parse_metal(ctx, scene_data, material_map);
  } else if (type == "refractive:") {
    parse_refractive(ctx, scene_data, material_map);
  } else if (type == "sphere:") {
    parse_sphere(ctx, scene_data, material_map);
  } else if (type == "cylinder:") {
    parse_cylinder(ctx, scene_data, material_map);
  } else {
    throw std::runtime_error("Unknown scene entity: " + type);
  }
}

// Parses a single line of the scene file; blank lines and # comments are
// skipped.
void process_scene_line(
    std::string const &line, int line_number, render::SceneData &scene_data,
    std::map<std::string, render::material::Material *> &material_map) {
  std::string trimmed_line = line;
  trimmed_line.erase(0, trimmed_line.find_first_not_of(" \t"));
  if (trimmed_line.empty() or trimmed_line[0] == '#') {
    return;
  }

  std::stringstream ss_line(trimmed_line);
  std::string type;
  ss_line >> type;

  std::string const full_line_for_error = trimmed_line;
  ParseContext ctx{&ss_line, &type, &full_line_for_error};

  try {
    dispatch_scene_entity_parser(ctx, scene_data, material_map);
  } catch (std::exception const &e) {
    throw std::runtime_error(
        "Line " + std::to_string(line_number) + ": " + e.what() +
        "\n Content: \"" + line + "\"");
  }
}

} // namespace

namespace render {

[[nodiscard]] RenderConfig ConfigParser::parse(std::string const &filename) {
  std::ifstream file(filename);
  if (not file.is_open()) {
    throw std::runtime_error("Could not open file: " + filename);
  }

  RenderConfig config;
  std::string line;
  int line_number = 0;

  while (std::getline(file, line)) {
    line_number++;
    line.erase(0, line.find_first_not_of(" \t"));

    if (line.empty() or line[0] == '#') {
      continue;
    }
    process_line(config, line, line_number);
  }

  file.close();
  return config;
}

[[nodiscard]] SceneData SceneParser::parse(std::string const &filename) {
  std::ifstream file(filename);
  if (not file.is_open()) {
    throw std::runtime_error("Could not open scene file: " + filename);
  }

  SceneData scene_data;
  std::map<std::string, material::Material *> material_map;

  std::string line;
  int line_number = 0;

  while (std::getline(file, line)) {
    line_number++;
    process_scene_line(line, line_number, scene_data, material_map);
  }

  file.close();
  return scene_data;
}

} // namespace render
