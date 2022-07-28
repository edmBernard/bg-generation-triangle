
#pragma once

#include "geometry.hpp"

#include <fmt/format.h>
#include <spdlog/spdlog.h>

#include <filesystem>
#include <fstream>

namespace svg {

struct Color {
  int r;
  int g;
  int b;
  float opacity = 1;

  Color(int r, int g, int b, float opacity = 1)
      : r(r), g(g), b(b), opacity(opacity) {
  }
  Color(uint32_t hexColor, float opacity = 1)
      : r((hexColor & 0xFF0000) >> 16), g((hexColor & 0x00FF00) >> 8), b(hexColor & 0x0000FF), opacity(opacity) {
  }
};

// Color operation use for color interpolation
Color operator+(const Color &c1, const Color &c2) {
  return {c1.r + c2.r, c1.g + c2.g, c1.b + c2.b};
}
Color operator-(const Color &c1, const Color &c2) {
  return {c1.r - c2.r, c1.g - c2.g, c1.b - c2.b};
}
Color operator*(float value, const Color &c) {
  return {int(value * c.r), int(value * c.g), int(value * c.b)};
}
float norm(const Color &c1) {
  return c1.r * c1.r + c1.g * c1.g + c1.b * c1.b;
}

struct Fill : public Color {
  Fill(Color rgb)
      : Color(rgb) {
  }
  Fill(uint32_t hexColor)
      : Color(hexColor) {
  }
};

struct Stroke : public Color {
  float width = 0;
  Stroke(Color rgb, float width)
      : Color(rgb), width(width) {
  }
  Stroke(uint32_t hexColor, float width)
      : Color(hexColor), width(width) {
  }
};

namespace details {

std::string to_draw(const Triangle &tr) {
  return fmt::format("M {} {} L {} {} L {} {} Z", tr.vertices[2].x, tr.vertices[2].y, tr.vertices[0].x, tr.vertices[0].y, tr.vertices[1].x, tr.vertices[1].y);
}

std::string to_draw(const Quadrilateral &tr) {
  return fmt::format("M {} {} L {} {} L {} {} L {} {} Z", tr.vertices[0].x, tr.vertices[0].y, tr.vertices[1].x, tr.vertices[1].y, tr.vertices[3].x, tr.vertices[3].y, tr.vertices[2].x, tr.vertices[2].y);
}

std::string to_draw(const Bezier &bz) {
  return fmt::format("M {} {} C {} {}, {} {}, {} {}",
                     bz.points[0].x, bz.points[0].y,
                     bz.points[1].x, bz.points[1].y,
                     bz.points[2].x, bz.points[2].y,
                     bz.points[3].x, bz.points[3].y);
}

std::string to_style(std::optional<Fill> fill) {
  return fill ? fmt::format("fill:rgb({},{},{})", fill->r, fill->g, fill->b) : "fill:none";
}

std::string to_style(std::optional<Stroke> stroke) {
  return stroke ? fmt::format("stroke:rgb({},{},{});stroke-width:{};stroke-opacity:{};stroke-linecap:butt;stroke-linejoin:round", stroke->r, stroke->g, stroke->b, stroke->width, stroke->opacity) : "";
}

} // namespace details

class Document {
public:
  Document(int canvasWidth, int canvasHeight, Color background)
      : canvasWidth(canvasWidth),
        canvasHeight(canvasHeight),
        backgroundColor(background) {
  }

  [[nodiscard]] bool save(std::filesystem::path filename) {

    std::ofstream out(filename);
    if (!out) {
      spdlog::error("Cannot open output file : {}.", filename.string());
      return false;
    }

    out << "<svg xmlns='http://www.w3.org/2000/svg' "
        << fmt::format("height='{height}' width='{width}' viewBox='0 0 {height} {width}'>\n", fmt::arg("height", canvasHeight), fmt::arg("width", canvasWidth))
        << fmt::format("<rect height='100%' width='100%' fill='rgb({},{},{})'/>\n", backgroundColor.r, backgroundColor.g, backgroundColor.b)
        << "<g id='surface1'>\n";

    out << content;

    out << "</g>\n</svg>\n";
    return true;
  }

  void addRaw(const std::string &raw) {
    content += raw;
  }

  void addBezier(const Bezier &bz, Stroke stroke) {
    const std::string s_stroke = fmt::format("stroke:rgb({},{},{});stroke-width:{};stroke-opacity:{};stroke-linecap:butt;stroke-linejoin:round",
                                             stroke.r, stroke.g, stroke.b, stroke.width, stroke.opacity);

    content += fmt::format("<path style='{};fill:none' d='{}'></path>\n ", s_stroke, details::to_draw(bz));
  }

  void addText(const std::string &text, Point position, Fill textColor) {
    const std::string s_fill = fmt::format("fill:rgb({},{},{})", textColor.r, textColor.g, textColor.b);
    content += fmt::format("<text style='{}' x={} y={} font-size='0.5em' dy='0.25em'>{}</text>\n", s_fill, position.x, position.y, text);
  }


  template <typename T, typename Lambda = std::function<bool(typename T::value_type)>>
  void addPath(const T &shapes, std::optional<Fill> fill, std::optional<Stroke> stroke, Lambda func = [](const typename T::value_type &) { return true; }) {
    std::string s_path;
    for (auto &elem : shapes) {
      if (func(elem)) {
        s_path += details::to_draw(elem) + " ";
      }
    }
    content += fmt::format("<path style='{};{}' d='{}'></path>\n", details::to_style(fill), details::to_style(stroke), s_path);
  }


private:
  int canvasWidth;
  int canvasHeight;
  Color backgroundColor;
  std::string content = "";
};

} // namespace svg
