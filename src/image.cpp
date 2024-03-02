#include <stdexcept>

#include "image.h"

Point2 ndc_to_raster(const Vec3& ndc_pos, int width, int height)
{
    // The top is zero and bottom is height-1 on the grid format used, so we
    // flip the y coordinate.
    return Point2(
        (int)roundf(((ndc_pos.x() + 1.0f) / 2.0f) * (float)width),
        (int)roundf((1.0f - (ndc_pos.y() + 1.0f) / 2.0f) * (float)height));
}

Image::Image(int width, int height)
    : w(width)
    , h(height)
{
    if (width <= 0 || height <= 0)
        throw std::runtime_error("Invalid image dimensions");
    grid = new Colour[(size_t)width * (size_t)height];
}

Image::~Image()
{
    delete[] grid;
}

bool Image::is_inside(int x, int y)
{
    return x < width() && y < height() && x >= 0 && y >= 0;
}

Colour& Image::get(int x, int y)
{
    if (!is_inside(x, y))
        throw std::runtime_error("Out of bounds");
    return grid[x + width() * y];
}

std::string Image::to_ppm() const
{
    std::string res = "P3\n";
    res += std::to_string(width()) + " " + std::to_string(height()) + "\n";
    res += "255\n";

    for (size_t i = 0; i < width() * height(); i++) {
        Colour& c = grid[i];
        res += std::to_string(Colour::to_byte(c.r)) + " " +
               std::to_string(Colour::to_byte(c.g)) + " " +
               std::to_string(Colour::to_byte(c.b)) + "\n";
    }

    return res;
}