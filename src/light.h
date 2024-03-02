#pragma once

#include "algebra.h"
#include "colour.h"

class PointLight
{
  public:
    PointLight(const Vec3& position, const Colour& colour, float attenuation)
        : pos(position)
        , col(colour)
        , attenuation(attenuation)
    {
    }

    Vec3& position() { return pos; }
    const Vec3& position() const { return pos; }
    Colour& colour() { return col; }
    const Colour& colour() const { return col; }
    float& get_attenuation() { return attenuation; }
    const float& get_attenuation() const { return attenuation; }

  private:
    Vec3 pos;
    Colour col;
    float attenuation;
};