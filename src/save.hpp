//
//  https://github.com/edmBernard/bg-generation-penrose
//
//  Created by Erwan BERNARD on 11/09/2021.
//
//  Copyright (c) 2021 Erwan BERNARD. All rights reserved.
//  Distributed under the Apache License, Version 2.0. (See accompanying
//  file LICENSE or copy at http://www.apache.org/licenses/LICENSE-2.0)
//

#pragma once

#include <penrose.hpp>

#include <spdlog/spdlog.h>

#include <fstream>
#include <random>
#include <vector>

namespace svg {
namespace details {

std::string to_path(const Triangle &tr) {
  return fmt::format("M {} {} L {} {} L {} {} Z", tr.vertices[2].x, tr.vertices[2].y, tr.vertices[0].x, tr.vertices[0].y, tr.vertices[1].x, tr.vertices[1].y);
}
std::string to_path(const Quadrilateral &tr) {
  return fmt::format("M {} {} L {} {} L {} {} L {} {} Z", tr.vertices[0].x, tr.vertices[0].y, tr.vertices[1].x, tr.vertices[1].y, tr.vertices[3].x, tr.vertices[3].y, tr.vertices[2].x, tr.vertices[2].y);
}
} // namespace details

struct RGB {
  int r;
  int g;
  int b;
};

struct Fill {
  int r;
  int g;
  int b;

  Fill(int r, int g, int b)
      : r(r), g(g), b(b) {
  }

  Fill(RGB rgb)
      : r(rgb.r), g(rgb.g), b(rgb.b) {
  }
};

struct Strockes {
  int r;
  int g;
  int b;
  float width;

  Strockes(int r, int g, int b, float width)
      : r(r), g(g), b(b), width(width) {
  }

  Strockes(RGB rgb, float width)
      : r(rgb.r), g(rgb.g), b(rgb.b), width(width) {
  }
};

template <typename T, typename Lambda = std::function<bool(T)>>
std::string to_path(
    const std::vector<T> &shape, std::optional<Fill> fill, std::optional<Strockes> strockes, Lambda func = [](const T &) { return true; }) {
  std::string s_path;
  for (auto &tr : shape) {
    if (func(tr)) {
      s_path += details::to_path(tr) + " ";
    }
  }
  std::string s_fill = fill ? fmt::format("fill:rgb({},{},{})", fill->r, fill->g, fill->b) : "fill:none";
  std::string s_strockes = strockes ? fmt::format("stroke:rgb({},{},{});stroke-width:{};stroke-linecap:butt;stroke-linejoin:round", strockes->r, strockes->g, strockes->b, strockes->width) : "";
  return fmt::format("<path style='{};{}' d='{}'></path>\n", s_fill, s_strockes, s_path);
}

[[nodiscard]] bool saveTiling(const std::string &filename, const std::vector<penrose::PenroseTriangle> &triangles, int canvasSize,
                              RGB rgb1, RGB rgb2, RGB background) {

  std::ofstream out(filename);
  if (!out) {
    spdlog::error("Cannot open output file : {}", filename);
    return false;
  }

  const float strokesWidth = std::sqrt(norm(triangles[0].vertices[0] - triangles[0].vertices[1])) / 15.0f;

  out << "<svg xmlns='http://www.w3.org/2000/svg' "
      << fmt::format("height='{size}' width='{size}' viewBox='0 0 {size} {size}'>\n", fmt::arg("size", canvasSize))
      << fmt::format("<rect height='100%' width='100%' fill='rgb({},{},{})'/>\n", background.r, background.g, background.b)
      << "<g id='surface1'>\n";
  out << to_path(triangles, Fill{rgb1}, Strockes{0, 0, 0, strokesWidth}, [](const penrose::PenroseTriangle &tr) { return isSmall(tr.color); });
  out << to_path(triangles, Fill{rgb2}, Strockes{0, 0, 0, strokesWidth}, [](const penrose::PenroseTriangle &tr) { return !isSmall(tr.color); });
  out << "</g>\n</svg>\n";
  return true;
}

[[nodiscard]] bool saveTiling(const std::string &filename, const std::vector<penrose::PenroseTriangle> &quad, int canvasSize,
                              RGB rgb1, RGB rgb2, RGB background, int threshold) {

  std::ofstream out(filename);
  if (!out) {
    spdlog::error("Cannot open output file : {}.", filename);
    return false;
  }
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<> distrib(0, 10);

  const float strokesWidth = std::sqrt(norm(quad[0].vertices[0]-quad[0].vertices[1])) / 15.0f;

  out << "<svg xmlns='http://www.w3.org/2000/svg' "
      << fmt::format("height='{size}' width='{size}' viewBox='0 0 {size} {size}'>\n", fmt::arg("size",canvasSize))
      << fmt::format("<rect height='100%' width='100%' fill='rgb({},{},{})'/>\n", background.r, background.g, background.b)
      << "<g id='surface1'>\n";
  out << to_path(quad, Fill{rgb1}, {}, [&](const penrose::PenroseTriangle &tr) { return isSmall(tr.color) ? distrib(gen) >= threshold : false; });
  out << to_path(quad, Fill{rgb2}, {}, [&](const penrose::PenroseTriangle &tr) { return !isSmall(tr.color) ? distrib(gen) >= threshold : false; });
  out << to_path(quad, {}, Strockes{0, 0, 0, strokesWidth});
  out << "</g>\n</svg>\n";
  return true;
}

[[nodiscard]] bool saveTiling(const std::string &filename,
                              const std::vector<penrose::PenroseTriangle> &quadsStep1,
                              const std::vector<penrose::PenroseTriangle> &quadsStep2,
                              int canvasSize,
                              RGB rgbSmall1, RGB rgbBig1,
                              RGB rgbSmall2, RGB rgbBig2,
                              RGB background, int threshold) {

  std::ofstream out(filename);
  if (!out) {
    spdlog::error("Cannot open output file : {}.", filename);
    return false;
  }
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<> distrib(0, 10);

  out << "<svg xmlns='http://www.w3.org/2000/svg' "
      << fmt::format("height='{size}' width='{size}' viewBox='0 0 {size} {size}'>\n", fmt::arg("size", canvasSize))
      << fmt::format("<rect height='100%' width='100%' fill='rgb({},{},{})'/>\n", background.r, background.g, background.b)
      << "<g id='surface1'>\n";

  // orange
  // const RGB red1 {0xDB, 0x56, 0x00};
  // const RGB red2 {0x8C, 0x29, 0x01};
  // const RGB red3 {0x69, 0x15, 0x00};
  // const RGB red4 {0x42, 0x05, 0x00};
  // const RGB red5 {0x26, 0x00, 0x06};

  // nice blue not enough saturated / dark
  // const RGB red1 {0x60, 0xCD, 0xDB};
  // const RGB red2 {0x42, 0x81, 0xA1};
  // const RGB red3 {0x2D, 0x55, 0x80};
  // const RGB red4 {0x16, 0x30, 0x59};
  // const RGB red5 {0x06, 0x17, 0x38};

  // blue but too light
  // const RGB red1 {0x73, 0xF3, 0xFF};
  // const RGB red2 {0x60, 0xBC, 0xEB};
  // const RGB red3 {0x48, 0x89, 0xCF};
  // const RGB red4 {0x29, 0x59, 0xA6};
  // const RGB red5 {0x0B, 0x29, 0x66};

  const RGB red1 {222, 10, 20};
  const RGB red2 {171, 10, 20};
  const RGB red3 {113, 10, 20};
  const RGB red4 {72, 10, 20};
  const RGB red5 {24, 10, 10};
  const float strokesWidthStep2 = std::sqrt(norm(quadsStep2[0].vertices[0] - quadsStep2[0].vertices[1])) / 15.0f;
  const float strokesWidthStep1 = std::sqrt(norm(quadsStep1[0].vertices[0] - quadsStep1[0].vertices[1])) / 20.0f;
  out << to_path(quadsStep1, Fill{red5}, {}, [&](const penrose::PenroseTriangle &tr) { return tr.flag == 0; });
  out << to_path(quadsStep1, Fill{red5}, {}, [&](const penrose::PenroseTriangle &tr) { return tr.flag == 1; });
  out << to_path(quadsStep1, Fill{red5}, {}, [&](const penrose::PenroseTriangle &tr) { return tr.flag == 2; });
  out << to_path(quadsStep1, Fill{red4}, {}, [&](const penrose::PenroseTriangle &tr) { return tr.flag == 3; });
  out << to_path(quadsStep1, Fill{red4}, {}, [&](const penrose::PenroseTriangle &tr) { return tr.flag == 4; });
  out << to_path(quadsStep1, Fill{red3}, {}, [&](const penrose::PenroseTriangle &tr) { return tr.flag == 5; });
  out << to_path(quadsStep1, Fill{red3}, {}, [&](const penrose::PenroseTriangle &tr) { return tr.flag == 6; });
  out << to_path(quadsStep1, Fill{red2}, {}, [&](const penrose::PenroseTriangle &tr) { return tr.flag == 7; });
  out << to_path(quadsStep1, Fill{red2}, {}, [&](const penrose::PenroseTriangle &tr) { return tr.flag == 8; });
  out << to_path(quadsStep1, Fill{red1}, {}, [&](const penrose::PenroseTriangle &tr) { return tr.flag == 9; });
  out << to_path(quadsStep1, Fill{red1}, {}, [&](const penrose::PenroseTriangle &tr) { return tr.flag == 10; });

  out << to_path(quadsStep2, Fill{red5}, {}, [&](const penrose::PenroseTriangle &tr) { return tr.flag == 0 ? distrib(gen) >= threshold : false; });
  out << to_path(quadsStep2, Fill{red5}, {}, [&](const penrose::PenroseTriangle &tr) { return tr.flag == 1 ? distrib(gen) >= threshold : false; });
  out << to_path(quadsStep2, Fill{red5}, {}, [&](const penrose::PenroseTriangle &tr) { return tr.flag == 2 ? distrib(gen) >= threshold : false; });
  out << to_path(quadsStep2, Fill{red5}, {}, [&](const penrose::PenroseTriangle &tr) { return tr.flag == 3 ? distrib(gen) >= threshold : false; });
  out << to_path(quadsStep2, Fill{red4}, {}, [&](const penrose::PenroseTriangle &tr) { return tr.flag == 4 ? distrib(gen) >= threshold : false; });
  out << to_path(quadsStep2, Fill{red4}, {}, [&](const penrose::PenroseTriangle &tr) { return tr.flag == 5 ? distrib(gen) >= threshold : false; });
  out << to_path(quadsStep2, Fill{red3}, {}, [&](const penrose::PenroseTriangle &tr) { return tr.flag == 6 ? distrib(gen) >= threshold : false; });
  out << to_path(quadsStep2, Fill{red3}, {}, [&](const penrose::PenroseTriangle &tr) { return tr.flag == 7 ? distrib(gen) >= threshold : false; });
  out << to_path(quadsStep2, Fill{red2}, {}, [&](const penrose::PenroseTriangle &tr) { return tr.flag == 8 ? distrib(gen) >= threshold : false; });
  out << to_path(quadsStep2, Fill{red2}, {}, [&](const penrose::PenroseTriangle &tr) { return tr.flag == 9 ? distrib(gen) >= threshold : false; });
  out << to_path(quadsStep2, Fill{red2}, {}, [&](const penrose::PenroseTriangle &tr) { return tr.flag == 10 ? distrib(gen) >= threshold : false; });

  out << to_path(quadsStep1, {}, Strockes{0, 0, 0, strokesWidthStep1});
  // out << to_path(quadsStep2, {}, Strockes{0, 0, 0, strokesWidthStep2});
  out << "</g>\n</svg>\n";
  return true;
}

} // namespace svg