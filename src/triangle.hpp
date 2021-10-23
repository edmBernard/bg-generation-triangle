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

#include <algorithm>
#include <iterator>
#include <stdexcept>
#include <vector>
#include <random>

namespace draw {

enum class TriangleKind {
  kCentral,
  kBorder,
};


struct ColoredTriangle : Triangle {
  TriangleKind kind;
  int flag;

  ColoredTriangle(TriangleKind kind, Point A, Point B, Point C, int flag = false)
      : Triangle(A, B, C), kind(kind), flag(flag) {
  }

};

std::string to_string(const ColoredTriangle &triangle) {
  return fmt::format("{}, {}, {}, {}", triangle.kind, to_string(triangle.vertices[0]), to_string(triangle.vertices[1]), to_string(triangle.vertices[2]));
}

std::vector<ColoredTriangle> deflate(const ColoredTriangle &triangle) {
  const Point A = triangle.vertices[0];
  const Point B = triangle.vertices[1];
  const Point C = triangle.vertices[2];

  const Point a = A + ((B - A) + (C - A)) / 2.;
  const Point b = B + ((A - B) + (C - B)) / 2.;
  const Point c = C + ((A - C) + (B - C)) / 2.;
  return std::vector<ColoredTriangle>{
      {TriangleKind::kBorder, A, b, c, triangle.flag},
      {TriangleKind::kBorder, B, c, a, triangle.flag},
      {TriangleKind::kBorder, C, a, b, triangle.flag},
      {TriangleKind::kCentral, a, b, c, triangle.flag}};
}

std::vector<ColoredTriangle> deflate(const std::vector<ColoredTriangle> &triangles) {
  std::vector<ColoredTriangle> newList;
  for (const auto &triangle : triangles) {
    const auto small = deflate(triangle);
    std::move(small.begin(), small.end(), std::back_inserter(newList));
  }
  return newList;
}

void setRandomFlag(std::vector<ColoredTriangle> &quadrilaterals) {
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<> distrib(0, 10);
  for (auto &quad : quadrilaterals) {
    quad.flag = distrib(gen);
  }
}



} // namespace draw