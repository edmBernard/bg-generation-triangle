//
//  https://github.com/edmBernard/bg-generation-penrose
//
//  Created by Erwan BERNARD on 11/09/2021.
//
//  Copyright (c) 2021 Erwan BERNARD. All rights reserved.
//  Distributed under the Apache License, Version 2.0. (See accompanying
//  file LICENSE or copy at http://www.apache.org/licenses/LICENSE-2.0)
//

#include <geometry.hpp>
#include <penrose.hpp>
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
  options.positional_help("output [level]").show_positional_help();

  // clang-format off
  options.add_options()
    ("h,help", "Print help")
    ("l,level", "Number of subdivision done", cxxopts::value<int>()->default_value("11"))
    ("o,output", "Output filename (.svg)", cxxopts::value<std::string>())
    ("rhombus", "Use Rhombus (P3) form otherwise it use Kite and Dart (P2)", cxxopts::value<bool>())
    ("step", "Step of the 2 color", cxxopts::value<int>()->default_value("0"))
    ("threshold", "Threshold for holes [0, 10] (0: no holes)", cxxopts::value<int>()->default_value("7"))
    ;
  // clang-format on
  options.parse_positional({"output", "level", "step"});
  auto clo = options.parse(argc, argv);

  if (clo.count("help")) {
    fmt::print("{}", options.help());
    return EXIT_SUCCESS;
  }
  if (!clo.count("output")) {
    spdlog::error("Output filename is required");
    fmt::print("{}", options.help());
    return EXIT_FAILURE;
  }

  const int level = clo["level"].as<int>();
  const int step = clo["step"].as<int>();
  const int threshold = clo["threshold"].as<int>();
  const std::string filename = clo["output"].as<std::string>();

  // =================================================================================================
  // Code

  using namespace penrose;

  auto start_temp = std::chrono::high_resolution_clock::now();

  std::vector<PenroseTriangle> tiling;

  const int canvasSize = 2000;
  const float radius = canvasSize;
  const Point center = canvasSize / 2.f * Point(1, 1);
  // Tiling initialisation

  for (int i = 0, sign = -1; i < 6; ++i, sign *= -1) {
    const float phi1 = (2 * i - sign) * pi / 6;
    const float phi2 = (2 * i + sign) * pi / 6;

    tiling.emplace_back(
        TriangleKind::kDart,
        radius * Point(cos(phi1), sin(phi1)) + center,
        Point(0, 0) + center,
        radius * Point(cos(phi2), sin(phi2)) + center);
  }

  for (int l = 0; l < level; ++l) {
    tiling = deflate(tiling);
  }
  std::vector<PenroseTriangle> smallTiling = deflate(tiling);

  setRandomFlag(tiling);
  setRandomFlag(smallTiling);

  if (!svg::saveTiling(filename, tiling, smallTiling, canvasSize,
                       svg::RGB{255, 140, 140}, svg::RGB{100, 70, 70},
                       svg::RGB{30, 30, 140}, svg::RGB{30, 140, 30},
                       svg::RGB{30, 30, 30}, 9)) {
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