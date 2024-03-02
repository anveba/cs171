#pragma once

#include "image.h"
#include "scene.h"

class WireframeRenderer
{
  public:
    WireframeRenderer();

    virtual void render(Image& image, const Scene& scene);
};