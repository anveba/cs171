#pragma once

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wignored-attributes"
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-enum-enum-conversion"
#include <Eigen/Dense>
#pragma GCC diagnostic pop
#pragma GCC diagnostic pop
#pragma GCC diagnostic pop

typedef Eigen::Matrix3f Mat3;
typedef Eigen::Matrix4f Mat4;
typedef Eigen::Vector3f Vec3;
typedef Eigen::Vector4f Vec4;
typedef Eigen::Vector2i Point2;

Mat4 translation(Vec3 t);
Mat4 rotation(Vec3 axis, float angle);
Mat4 scaling(Vec3 s);
Mat4 projection(
    float near,
    float far,
    float left,
    float right,
    float top,
    float bottom);