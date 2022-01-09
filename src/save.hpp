//
//  https://github.com/edmBernard/bg-generation-triangle
//
//  Created by Erwan BERNARD on 11/09/2021.
//
//  Copyright (c) 2021 Erwan BERNARD. All rights reserved.
//  Distributed under the Apache License, Version 2.0. (See accompanying
//  file LICENSE or copy at http://www.apache.org/licenses/LICENSE-2.0)
//

#pragma once

#include <geometry.hpp>
#include <libsvg.hpp>

#include <spdlog/spdlog.h>

#include <fstream>
#include <random>
#include <string>
#include <vector>


std::vector<svg::Color> getColorPalette(int index) {
  switch (index) {
  case 0:
    // blue but too light
    return {
        {0x73F3FF},
        {0x60BCEB},
        {0x4889CF},
        {0x2959A6},
        {0x0B2966}};
  case 1:
    // nice blue not enough saturated / dark
    return {
        {0x60CDDB},
        {0x4281A1},
        {0x2D5580},
        {0x163059},
        {0x061738}};
  case 2:
    // orange
    return {
        {0xDB5600},
        {0x8C2901},
        {0x691500},
        {0x420500},
        {0x260006}};
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


std::vector<svg::Color> getColorPalette(svg::Color colorBegin, svg::Color colorEnd) {
  const svg::Color colorDirection = colorEnd - colorBegin;
  const float directionNorm = norm(colorDirection);
  return {
    colorBegin,
    colorBegin + (1/4.) * colorDirection,
    colorBegin + (2/4.) * colorDirection,
    colorBegin + (3/4.) * colorDirection,
    colorEnd
  };
};


template <typename Geometry>
[[nodiscard]] bool saveTiling(const std::string &filename,
                              const std::vector<Geometry> &bigGeometry,
                              const std::vector<Geometry> &smallGeometry,
                              int canvasSize,
                              std::vector<svg::Color> palette, bool haveStrokes, int threshold) {

  svg::Document doc(canvasSize, canvasSize, 0x000000);

  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<> distrib(0, 10);

  {
    std::vector<int> colorRepartitionBig{2, 2, 2, 2, 3};
    int count = 0;
    for (int c = 0; c < palette.size(); ++c) {
      for (int i = 0; i < colorRepartitionBig[c]; ++i, ++count) {
        doc.addPath(bigGeometry, svg::Fill{palette[c]}, {}, [&](const Geometry &tr) { return tr.flag == count; });
      }
    }
  }

  {
    std::vector<int> colorRepartitionSmall{0, 3, 2, 2, 4};
    int count = 0;
    for (int c = 0; c < palette.size(); ++c) {
      for (int i = 0; i < colorRepartitionSmall[c]; ++i, ++count) {
        doc.addPath(smallGeometry, svg::Fill{palette[c]}, {}, [&](const Geometry &tr) { return tr.flag == count ? distrib(gen) >= threshold : false; });
      }
    }
  }

  if (haveStrokes) {
    const float strokeWidth = norm(bigGeometry[0].vertices[0] - bigGeometry[0].vertices[1]) / 20.0f;
    doc.addPath(bigGeometry, {}, svg::Stroke{{0, 0, 0}, strokeWidth});
  }

  return doc.save(filename);
}
