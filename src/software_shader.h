#pragma once

#include "material.h"
#include "scene.h"
#include "surface_point.h"

class SoftwareShader
{
  public:
    virtual Colour shade(const SurfacePoint& surface,
                         const PhongMaterial& material,
                         const Scene& scene) const = 0;
    virtual bool per_pixel_shading() const = 0;
};