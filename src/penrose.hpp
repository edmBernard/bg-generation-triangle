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

namespace penrose {

constexpr float pi = 3.14159265358979f;
constexpr float goldenRatio = 1.618033988749895f;
constexpr int levelToFix = 6;

enum class TriangleKind {
  kKite,
  kDart,
  kRhombsCyan,
  kRhombsViolet
};

inline bool isSmall(TriangleKind kind) {
  return kind == TriangleKind::kKite || kind == TriangleKind::kRhombsCyan;
}

struct PenroseTriangle : Triangle {
  TriangleKind color;
  int flag;

  PenroseTriangle(TriangleKind color, Point A, Point B, Point C, int flag = false)
      : Triangle(A, B, C), color(color), flag(flag) {
  }

};

std::string to_string(const PenroseTriangle &triangle) {
  return fmt::format("{}, {}, {}, {}", triangle.color, to_string(triangle.vertices[0]), to_string(triangle.vertices[1]), to_string(triangle.vertices[2]));
}

std::vector<PenroseTriangle> deflate(const PenroseTriangle &triangle) {
  const Point A = triangle.vertices[0];
  const Point B = triangle.vertices[1];
  const Point C = triangle.vertices[2];

  const Point a = A + ((B - A) + (C - A)) / 2.;
  const Point b = B + ((A - B) + (C - B)) / 2.;
  const Point c = C + ((A - C) + (B - C)) / 2.;
  return std::vector<PenroseTriangle>{
      {TriangleKind::kDart, A, b, c, triangle.flag},
      {TriangleKind::kDart, B, c, a, triangle.flag},
      {TriangleKind::kDart, C, a, b, triangle.flag},
      {TriangleKind::kKite, a, b, c, triangle.flag}};
}

std::vector<PenroseTriangle> deflate(const std::vector<PenroseTriangle> &triangles) {
  std::vector<PenroseTriangle> newList;
  for (const auto &triangle : triangles) {
    const auto small = deflate(triangle);
    std::move(small.begin(), small.end(), std::back_inserter(newList));
  }
  return newList;
}

void setRandomFlag(std::vector<PenroseTriangle> &quadrilaterals) {
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<> distrib(0, 10);
  for (auto &quad : quadrilaterals) {
    quad.flag = distrib(gen);
  }
}



} // namespace penrose