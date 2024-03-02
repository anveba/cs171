#pragma once

#include <cstddef>
#include <string>

#include "algebra.h"
#include "colour.h"

Point2 ndc_to_raster(const Vec3& ndc_pos, int width, int height);

// Represents a grid of colours.
class Image
{
  public:
    Image(int width, int height);
    ~Image();

    Image(Image const&) = delete;
    void operator=(Image const&) = delete;

    bool is_inside(int x, int y);
    Colour& get(int x, int y);

    Colour& get_unchecked(int x, int y) { return grid[x + width() * y]; }

    std::string to_ppm() const;

    int width() const { return w; }
    int height() const { return h; }

  private:
    int w, h;
    Colour* grid;
};