#include <stdexcept>

#include "depth_buffer.h"

DepthBuffer::DepthBuffer(int width, int height)
    : w(width)
    , h(height)
{
    if (width <= 0 || height <= 0)
        throw std::runtime_error("Invalid depth buffer dimensions");
    grid = new float[(size_t)width * (size_t)height];
}

DepthBuffer::~DepthBuffer()
{
    delete[] grid;
}

bool DepthBuffer::is_inside(int x, int y)
{
    return x < width() && y < height() && x >= 0 && y >= 0;
}

float& DepthBuffer::get(int x, int y)
{
    if (!is_inside(x, y))
        throw std::runtime_error("Out of bounds");
    return grid[x + width() * y];
}

void DepthBuffer::clear(float value)
{
    for (size_t i = 0; i < (size_t)width() * height(); i++)
        grid[i] = value;
}