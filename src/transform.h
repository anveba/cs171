#pragma once

#include "algebra.h"

// Represents a transformation that can be made on a mesh.
// Holds one matrix: the combined transformation matrix.
class Transform
{
  public:
    Transform()
        : mat(Mat4::Identity())
    {
    }
    Transform(Mat4 transform)
        : mat(transform)
    {
    }

    Mat4& matrix() { return mat; }
    const Mat4& matrix() const { return mat; }

  private:
    Mat4 mat;
};