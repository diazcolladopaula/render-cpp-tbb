#include "camera.hpp"
#include "material.hpp"
#include "par_image.hpp"
#include "parsers.hpp"
#include <chrono>
#include <cstdlib>
#include <exception>
#include <iostream>
#include <map>
#include <memory>
#include <optional>
#include <span>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include <oneapi/tbb/global_control.h>
#include <oneapi/tbb/task_arena.h>

namespace {

constexpr int default_simple_grain = 32;
constexpr int default_auto_grain = 2;

// Parses an integer command-line argument, with a readable error message.
int parse_int_arg(std::string const &text, std::string const &name) {
  try {
    std::size_t pos = 0;
    int const value = std::stoi(text, &pos);
    if (pos != text.size()) {
      throw std::invalid_argument(text);
    }
    return value;
  } catch (std::exception const &) {
    throw std::invalid_argument("Invalid value for " + name + ": '" + text +
                                "' (expected an integer)");
  }
}

// Resolves the material name of every object to the actual material.
void link_materials(render::SceneData &scene_data) {
  std::map<std::string, std::shared_ptr<render::material::Material>>
      material_map;
  for (auto &mat : scene_data.materials) {
    material_map[mat->get_name()] = std::move(mat);
  }
  for (auto &obj : scene_data.objects) {
    auto it = material_map.find(obj->get_material_name());
    if (it != material_map.end()) {
      obj->set_material(it->second);
    }
  }
}

// Limits TBB to the requested number of threads (argument 4, optional).
// Without the argument TBB uses every available hardware thread.
void setup_thread_limit(std::span<char *> args, int &requested_threads,
                        std::optional<tbb::global_control> &thread_limit) {
  requested_threads = tbb::this_task_arena::max_concurrency();
  if (args.size() >= 5) {
    requested_threads = parse_int_arg(args[4], "threads");
    if (requested_threads < 1) {
      throw std::invalid_argument("Thread count must be at least 1");
    }
    thread_limit.emplace(tbb::global_control::max_allowed_parallelism,
                         static_cast<std::size_t>(requested_threads));
  }
}

render::PartitionPolicy parse_partition_policy(std::string const &policy_str) {
  if (policy_str == "auto") {
    return render::PartitionPolicy::Auto;
  }
  if (policy_str == "simple") {
    return render::PartitionPolicy::Simple;
  }
  if (policy_str == "static") {
    return render::PartitionPolicy::Static;
  }
  throw std::invalid_argument("Invalid partition policy: " + policy_str +
                              ". Valid options are: auto, simple, static");
}

// Reads the policy (argument 5) and grain size (argument 6), both optional.
render::RenderParams setup_render_params(std::span<char *> args) {
  render::RenderParams params;
  if (args.size() >= 6) {
    std::string const policy_str = args[5];
    params.policy = parse_partition_policy(policy_str);
    std::cerr << "  Partition policy: " << policy_str << "\n";
  } else {
    std::cerr << "  Partition policy: auto (default)\n";
  }

  if (args.size() >= 7) {
    params.grain_size = parse_int_arg(args[6], "grain size");
    if (params.grain_size < 0) {
      throw std::invalid_argument("Grain size must be non-negative");
    }
    // simple_partitioner needs an explicit grain size.
    if (params.policy == render::PartitionPolicy::Simple and
        params.grain_size == 0) {
      std::cerr << "  Warning: simple partitioner with grain size 0, using "
                << default_simple_grain << "\n";
      params.grain_size = default_simple_grain;
    }
    std::cerr << "  Grain size: " << params.grain_size << "\n";
  } else if (params.policy == render::PartitionPolicy::Simple) {
    params.grain_size = default_simple_grain;
    std::cerr << "  Grain size: " << default_simple_grain
              << " (default for simple)\n";
  } else {
    params.grain_size = default_auto_grain;
    std::cerr << "  Grain size: " << default_auto_grain << " (default)\n";
  }
  return params;
}

int compute_image_height(render::RenderConfig const &config) {
  return static_cast<int>(config.image_width /
                          (static_cast<double>(config.aspect_ratio_w) /
                           static_cast<double>(config.aspect_ratio_h)));
}

void save_rendered_image(render::RenderConfig const &config,
                         std::vector<int> const &image_data,
                         std::string const &output_file) {
  render::par::Image image(config.image_width, compute_image_height(config));
  image.set_data(image_data);
  image.save_ppm(output_file);
}

void print_usage(char const *program_name) {
  std::cerr
      << "Usage: " << program_name
      << " <config_file> <scene_file> <output_file> [threads] [policy] "
         "[grain_size]\n"
         "\nArguments:\n"
         "  config_file  Path to the configuration file\n"
         "  scene_file   Path to the scene description file\n"
         "  output_file  Path of the output PPM image\n"
         "  threads      Number of threads (optional, default: all available)\n"
         "  policy       Partitioning policy: auto, simple or static "
         "(optional, default: auto)\n"
         "  grain_size   Grain size (optional)\n"
         "                 simple: default "
      << default_simple_grain
      << "\n"
         "                 auto:   default "
      << default_auto_grain
      << "\n"
         "                 static: ignored\n";
}

} // namespace

int main(int argc, char **argv) {
  try {
    auto const args = std::span<char *>(argv, static_cast<std::size_t>(argc));
    if (args.size() < 4 or args.size() > 7) {
      std::cerr << "Error: Invalid number of arguments: " << (argc - 1)
                << "\n\n";
      print_usage(args[0]);
      return EXIT_FAILURE;
    }
    std::string const config_file = args[1];
    std::string const scene_file = args[2];
    std::string const output_file = args[3];

    std::cerr << "Render Configuration:\n";
    std::optional<tbb::global_control> thread_limit;
    int requested_threads = 0;
    setup_thread_limit(args, requested_threads, thread_limit);
    std::cerr << "  Threads: " << requested_threads << "\n";
    render::RenderParams const render_params = setup_render_params(args);

    render::RenderConfig const config =
        render::ConfigParser::parse(config_file);
    render::SceneData scene_data = render::SceneParser::parse(scene_file);
    link_materials(scene_data);

    render::Camera const camera(config);
    auto const start = std::chrono::high_resolution_clock::now();
    auto const image_data = camera.render(scene_data.objects, render_params);
    auto const end = std::chrono::high_resolution_clock::now();
    auto const duration =
        std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cerr << "Render time: " << duration.count() << " ms\n";

    save_rendered_image(config, image_data, output_file);
    return EXIT_SUCCESS;
  } catch (std::exception const &e) {
    std::cerr << "Error: " << e.what() << "\n";
    return EXIT_FAILURE;
  }
}
