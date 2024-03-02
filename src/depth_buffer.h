#pragma once

#include <cstddef>

class DepthBuffer
{
  public:
    DepthBuffer(int width, int height);
    ~DepthBuffer();

    DepthBuffer(DepthBuffer const&) = delete;
    void operator=(DepthBuffer const&) = delete;

    bool is_inside(int x, int y);
    float& get(int x, int y);
    void clear(float value);

    float& get_unchecked(int x, int y) { return grid[x + width() * y]; }

    int width() const { return w; }
    int height() const { return h; }

  private:
    int w, h;
    float* grid;
};