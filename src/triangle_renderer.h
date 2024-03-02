#pragma once

#include <type_traits>

#include "image.h"
#include "scene.h"
#include "software_shader.h"

// A class that handles rasterisation of triangles.
class TriangleRenderer
{
  public:
    // Renders a scene according to the given shading algorithm (for example Gouraud or Phong)
    void render(SoftwareShader* shader, Image& image, const Scene& scene);
};