#include "quaternion.h"

Quaternion::Quaternion(float s, Vec3 v)
    : s(s)
    , v(v)
{
}

Quaternion::Quaternion()
{
    s = 1;
    v = Vec3::Zero();
}

Mat4 Quaternion::to_rotation_matrix() const
{
    Quaternion unit_q = *this / norm();

    Mat4 rot;
    rot <<
        // Row 1
        1 - 2 * unit_q.v.y() * unit_q.v.y() - 2 * unit_q.v.z() * unit_q.v.z(),
        2 * (unit_q.v.x() * unit_q.v.y() - unit_q.v.z() * unit_q.s),
        2 * (unit_q.v.x() * unit_q.v.z() + unit_q.v.y() * unit_q.s),
        0,

        // Row 2
        2 * (unit_q.v.x() * unit_q.v.y() + unit_q.v.z() * unit_q.s),
        1 - 2 * unit_q.v.x() * unit_q.v.x() - 2 * unit_q.v.z() * unit_q.v.z(),
        2 * (unit_q.v.y() * unit_q.v.z() - unit_q.v.x() * unit_q.s),
        0,

        // Row 3
        2 * (unit_q.v.x() * unit_q.v.z() - unit_q.v.y() * unit_q.s),
        2 * (unit_q.v.y() * unit_q.v.z() + unit_q.v.x() * unit_q.s),
        1 - 2 * unit_q.v.x() * unit_q.v.x() - 2 * unit_q.v.y() * unit_q.v.y(),
        0,

        // Row 4
        0,
        0,
        0,
        1;

    return rot;
}

Quaternion Quaternion::from_rotation(Vec3 axis, float angle)
{
    return Quaternion(cosf(angle / 2.0f), axis.normalized() * sinf(angle / 2.0f));
}

float Quaternion::norm_squared() const
{
    return s * s + v.squaredNorm();
}

float Quaternion::norm() const
{
    return sqrtf(norm_squared());
}

Quaternion Quaternion::conjugate() const
{
    return Quaternion(s, -v);
}

Quaternion Quaternion::inverse() const
{
    return conjugate() / norm_squared();
}

Quaternion& Quaternion::operator+=(const Quaternion& q)
{
    *this = *this + q;
    return *this;
}

Quaternion& Quaternion::operator-=(const Quaternion& q)
{
    *this = *this - q;
    return *this;
}

Quaternion& Quaternion::operator*=(const Quaternion& q)
{
    *this = *this * q;
    return *this;
}

Quaternion& Quaternion::operator*=(float v)
{
    *this = *this * v;
    return *this;
}

Quaternion& Quaternion::operator/=(float v)
{
    *this = *this / v;
    return *this;
}

Quaternion operator+(const Quaternion& q1, const Quaternion& q2)
{
    return Quaternion(q1.s + q2.s, q1.v + q2.v);
}

Quaternion operator-(const Quaternion& q1, const Quaternion& q2)
{
    return Quaternion(q1.s - q2.s, q1.v - q2.v);
}

Quaternion operator*(const Quaternion& q1, const Quaternion& q2)
{
    return Quaternion(q1.s * q2.s - q1.v.dot(q2.v), q1.s * q2.v + q2.s * q1.v + q1.v.cross(q2.v));
}

Quaternion operator*(const Quaternion& q, float v)
{
    return Quaternion(v * q.s, v * q.v);
}
Quaternion operator*(float v, const Quaternion& q)
{
    return q * v;
}

Quaternion operator/(const Quaternion& q, float v)
{
    return q * (1.0f / v);
}

float& Quaternion::operator[](int i)
{
    if (i < 0 || i > 3)
        throw std::runtime_error("Out of bounds");
    if (i == 0)
        return s;
    return v[i - 1];
}

const float& Quaternion::operator[](int i) const
{
    if (i < 0 || i > 3)
        throw std::runtime_error("Out of bounds");
    if (i == 0)
        return s;
    return v[i - 1];
}