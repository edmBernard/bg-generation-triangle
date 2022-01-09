//
//  https://github.com/edmBernard/bg-generation-triangle
//
//  Created by Erwan BERNARD on 11/09/2021.
//
//  Copyright (c) 2021 Erwan BERNARD. All rights reserved.
//  Distributed under the Apache License, Version 2.0. (See accompanying
//  file LICENSE or copy at http://www.apache.org/licenses/LICENSE-2.0)
//

#include <geometry.hpp>
#include <triangle.hpp>
#include <libsvg.hpp>
#include <save.hpp>

#include <cxxopts.hpp>
#include <spdlog/cfg/env.h>
#include <spdlog/spdlog.h>

#include <chrono>
#include <fstream>
#include <vector>

int main(int argc, char *argv[]) try {

  spdlog::cfg::load_env_levels();

  // =================================================================================================
  // CLI
  cxxopts::Options options(argv[0], "Description");
  options.positional_help("output [level] [color]").show_positional_help();

  // clang-format off
  options.add_options()
    ("h,help", "Print help")
    ("l,level", "Number of subdivision done", cxxopts::value<int>()->default_value("11"))
    ("o,output", "Output filename (.svg)", cxxopts::value<std::string>())
    ("colorBegin", "First color in hex format", cxxopts::value<std::string>())
    ("colorEnd", "Last color in hex format", cxxopts::value<std::string>())
    ("color", "Color palette (0: blue1, 1:blue2, 2:red, 3:orange)", cxxopts::value<int>()->default_value("0"))
    ("strokes", "Draw Strokes", cxxopts::value<bool>())
    ;
  // clang-format on
  options.parse_positional({"output", "level", "color"});
  auto clo = options.parse(argc, argv);

  if (clo.count("help")) {
    fmt::print("{}", options.help());
    return EXIT_SUCCESS;
  }
  if (!clo.count("output")) {
    spdlog::error("Output filename is required");
    return EXIT_FAILURE;
  }
  if (clo.count("color") && (clo.count("colorBegin") || clo.count("colorEnd"))) {
    spdlog::error("Color and ColorBegin or ColorEnd are exclusive");
    return EXIT_FAILURE;
  }
  if (clo.count("colorBegin") ^ clo.count("colorEnd")) {
    spdlog::error("ColorBegin and ColorEnd should both be specified");
    return EXIT_FAILURE;
  }

  const int level = clo["level"].as<int>();
  const bool showStrokes = clo.count("strokes");
  const std::string filename = clo["output"].as<std::string>();

  // =================================================================================================
  // Code

  using namespace draw;

  auto start_temp = std::chrono::high_resolution_clock::now();

  std::vector<ColoredTriangle> tiling;

  const int canvasSize = 2000;
  const float radius = canvasSize;
  const Point center = canvasSize / 2.f * Point(1, 1);

  // Tiling initialisation
  tiling.emplace_back(TriangleKind::Border, radius * Point(1, 0), radius * Point(0, 0), radius * Point(0, 1));
  tiling.emplace_back(TriangleKind::Border, radius * Point(1, 0), radius * Point(1, 1), radius * Point(0, 1));

  for (int l = 0; l < level; ++l) {
    tiling = deflatePleasing(tiling);
  }

  setRandomFlag(tiling);

  // std::vector<svg::Color> colorPalette;
  // if (clo.count("color")) {
  //   colorPalette = getColorPalette(clo["color"].as<int>());
  // } else {
  //   const uint32_t colorBegin = std::stoul(clo["colorBegin"].as<std::string>(), nullptr, 16);
  //   const uint32_t colorEnd = std::stoul(clo["colorEnd"].as<std::string>(), nullptr, 16);
  //   colorPalette = getColorPalette(svg::Color(colorBegin), svg::Color(colorEnd));
  // }

  if (!saveTiling(filename, tiling, canvasSize, {}, showStrokes)) {
    spdlog::error("Failed to save in file");
    return EXIT_FAILURE;
  }

  std::chrono::duration<double, std::milli> elapsed_temp = std::chrono::high_resolution_clock::now() - start_temp;
  fmt::print("Execution time: {:.2f} ms \n", elapsed_temp.count());

  return EXIT_SUCCESS;

} catch (const cxxopts::OptionException &e) {
  spdlog::error("Parsing options : {}", e.what());
  return EXIT_FAILURE;

} catch (const std::exception &e) {
  spdlog::error("{}", e.what());
  return EXIT_FAILURE;
}
