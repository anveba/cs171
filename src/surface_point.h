#pragma once

#include "algebra.h"

struct SurfacePoint
{
    SurfacePoint(Vec3 world_pos, Vec3 normal)
        : world_position(world_pos)
        , normal(normal)
    {
    }

    Vec3 world_position, normal;
};