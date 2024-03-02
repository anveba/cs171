#include "animator.h"

#include <iostream>

Animator::Animator(const Animation& anim, bool loop)
    : animation(anim)
    , loop(loop)
{
}

Animator::Animator()
    : animation(Animation(1, { KeyFrame(0, Frame()) }))
    , loop(false)
{
}

Animator::~Animator()
{
}

static float clampf(float x, float lower, float upper)
{
    if (x > upper)
        return upper;
    else if (x < lower)
        return lower;
    return x;
}

static long long clampi(long long x, long long lower, long long upper)
{
    if (x > upper)
        return upper;
    else if (x < lower)
        return lower;
    return x;
}

static float modf(float x, float mod)
{
    return x - floorf(x / mod) * mod;
}

static long long modi(long long x, long long mod)
{
    return x - (long long)floorf((double)x / mod) * mod;
}

Frame Animator::interpolate_frame(float frame) const
{
    // Get key frame indices of the key frames used for interpolation. i1 and i2 are
    // the indices corresponding to p_i and p_i+1 in the lecture notes.
    size_t i1 = animation.index_of(loop ? modf(frame, animation.frame_count())
                                        : clampf(frame, 0.0f, animation.frame_count()));

    size_t i0 = loop ? modi(i1 - 1, animation.key_frame_count())
                     : clampi(i1 - 1, 0, animation.key_frame_count() - 1);
    size_t i2 = loop ? modi(i1 + 1, animation.key_frame_count())
                     : clampi(i1 + 1, 0, animation.key_frame_count() - 1);
    size_t i3 = loop ? modi(i1 + 2, animation.key_frame_count())
                     : clampi(i1 + 2, 0, animation.key_frame_count() - 1);

    // Avoid a division by zero
    if (i1 == i2)
        return animation.key_frame(i1).get_frame();

    // Calculate u and u vector
    size_t start_key_frame_num = animation.key_frame(i1).frame_number();
    size_t end_key_frame_num = animation.key_frame(i2).frame_number();
    if (end_key_frame_num < start_key_frame_num)
        end_key_frame_num += animation.frame_count();

    float u = (fmod(frame, animation.frame_count()) - start_key_frame_num) /
              (end_key_frame_num - start_key_frame_num);
    Vec4 u_vec(1.0f, u, u * u, u * u * u);

    // Calculate the B matrix
    Mat4 b_mat;
    b_mat << 0.0f, 2.0f, 0.0f, 0.0f,
        -1.0f, 0.0f, 1.0f, 0.0f,
        2.0f, -5.0f, 4.0f, -1.0f,
        -1.0f, 3.0f, -3.0f, 1.0f;
    b_mat *= 0.5f;

    Vec3 interpolated_pos, interpolated_scale;
    Quaternion interpolated_rot;

    // Get the frames to be used in interpolation
    const auto& f0 = animation.key_frame(i0).get_frame();
    const auto& f1 = animation.key_frame(i1).get_frame();
    const auto& f2 = animation.key_frame(i2).get_frame();
    const auto& f3 = animation.key_frame(i3).get_frame();

    // Interpolate positions
    for (int j = 0; j < 3; j++) {
        Vec4 p_vec(f0.position[j],
                   f1.position[j],
                   f2.position[j],
                   f3.position[j]);
        interpolated_pos[j] = u_vec.transpose() * b_mat * p_vec;
    }

    // Interpolate scales
    for (int j = 0; j < 3; j++) {
        Vec4 p_vec(f0.scale[j],
                   f1.scale[j],
                   f2.scale[j],
                   f3.scale[j]);
        interpolated_scale[j] = u_vec.transpose() * b_mat * p_vec;
    }

    // Interpolate rotations
    for (int j = 0; j < 4; j++) {
        Vec4 p_vec(f0.rotation[j],
                   f1.rotation[j],
                   f2.rotation[j],
                   f3.rotation[j]);
        interpolated_rot[j] = u_vec.transpose() * b_mat * p_vec;
    }
    interpolated_rot /= interpolated_rot.norm();

    return Frame(interpolated_pos, interpolated_rot, interpolated_scale);
}