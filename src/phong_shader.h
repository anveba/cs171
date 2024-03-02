#pragma once

#include "colour.h"
#include "software_shader.h"

class PhongShader : public SoftwareShader
{
  public:
    PhongShader(bool per_pixel_shading)
        : per_pixel(per_pixel_shading)
    {
    }

    Colour shade(const SurfacePoint& surface_point,
                 const PhongMaterial& material,
                 const Scene& scene) const override;

    bool per_pixel_shading() const override { return per_pixel; }

  private:
    bool per_pixel;
};