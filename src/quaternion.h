#pragma once

#include "algebra.h"

struct Quaternion
{
    Quaternion(float s, Vec3 v);
    Quaternion();

    float s;
    Vec3 v;

    Mat4 to_rotation_matrix() const;
    static Quaternion from_rotation(Vec3 axis, float angle);
    Quaternion conjugate() const;
    float norm_squared() const;
    float norm() const;
    Quaternion inverse() const;

    Quaternion& operator+=(const Quaternion& q);
    Quaternion& operator-=(const Quaternion& q);
    Quaternion& operator*=(const Quaternion& q);
    Quaternion& operator*=(float v);
    Quaternion& operator/=(float v);

    float& operator[](int i);
    const float& operator[](int i) const;
};

Quaternion operator+(const Quaternion& q1, const Quaternion& q2);
Quaternion operator-(const Quaternion& q1, const Quaternion& q2);
Quaternion operator*(const Quaternion& q1, const Quaternion& q2);
Quaternion operator*(const Quaternion& q, float v);
Quaternion operator*(float v, const Quaternion& q);
Quaternion operator/(const Quaternion& q, float v);