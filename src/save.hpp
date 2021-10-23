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

#include <geometry.hpp>

#include <spdlog/spdlog.h>

#include <fstream>
#include <random>
#include <string>
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

std::vector<RGB> getColorPalette(int index) {
  switch (index) {
  case 0:
    // blue but too light
    return {
        {0x73, 0xF3, 0xFF},
        {0x60, 0xBC, 0xEB},
        {0x48, 0x89, 0xCF},
        {0x29, 0x59, 0xA6},
        {0x0B, 0x29, 0x66}};
  case 1:
    // nice blue not enough saturated / dark
    return {
        {0x60, 0xCD, 0xDB},
        {0x42, 0x81, 0xA1},
        {0x2D, 0x55, 0x80},
        {0x16, 0x30, 0x59},
        {0x06, 0x17, 0x38}};
  case 2:
    // orange
    return {
        {0xDB, 0x56, 0x00},
        {0x8C, 0x29, 0x01},
        {0x69, 0x15, 0x00},
        {0x42, 0x05, 0x00},
        {0x26, 0x00, 0x06}};
  case 3:
    // red
    return {
        {222, 10, 20},
        {171, 10, 20},
        {113, 10, 20},
        {72, 10, 20},
        {24, 10, 10}};
  default:
    throw std::runtime_error("Unkown color palette index");
  };
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

template <typename Geometry>
[[nodiscard]] bool saveTiling(const std::string &filename,
                              const std::vector<Geometry> &bigGeometry,
                              const std::vector<Geometry> &smallGeometry,
                              int canvasSize,
                              std::vector<RGB> palette, bool strokes, int threshold) {

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
      << fmt::format("<rect height='100%' width='100%' fill='rgb({},{},{})'/>\n", 0, 0, 0)
      << "<g id='surface1'>\n";

  {
    std::vector<int> colorRepartitionBig{2, 2, 2, 2, 3};
    int count = 0;
    for (int c = 0; c < palette.size(); ++c) {
      for (int i = 0; i < colorRepartitionBig[c]; ++i, ++count) {
        out << to_path(bigGeometry, Fill{palette[c]}, {}, [&](const Geometry &tr) { return tr.flag == count; });
      }
    }
  }

  {
    std::vector<int> colorRepartitionSmall{0, 3, 2, 2, 4};
    int count = 0;
    for (int c = 0; c < palette.size(); ++c) {
      for (int i = 0; i < colorRepartitionSmall[c]; ++i, ++count) {
        out << to_path(smallGeometry, Fill{palette[c]}, {}, [&](const Geometry &tr) { return tr.flag == count ? distrib(gen) >= threshold : false; });
      }
    }
  }

  if (strokes) {
    const float strokes = std::sqrt(norm(bigGeometry[0].vertices[0] - bigGeometry[0].vertices[1])) / 20.0f;
    out << to_path(bigGeometry, {}, Strockes{0, 0, 0, strokes});
  }
  out << "</g>\n</svg>\n";
  return true;
}

} // namespace svg
