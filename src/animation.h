#pragma once
#include "algebra.h"
#include "quaternion.h"
#include "transform.h"
#include <vector>

struct Frame
{
  public:
    Frame()
    {
        scale = Vec3(1.0f, 1.0f, 1.0f);
    }

    Frame(Vec3 position, Quaternion rotation, Vec3 scale)
        : position(position)
        , scale(scale)
        , rotation(rotation)
    {
    }
    ~Frame() {}

    Vec3 position, scale;
    Quaternion rotation;

    Transform to_transform() const
    {
        return Transform(translation(position) * rotation.to_rotation_matrix() * scaling(scale));
    }

  private:
};

class KeyFrame
{
  public:
    KeyFrame(size_t frame_number, Frame frame)
        : frame_num(frame_number)
        , frame(frame)
    {
    }
    ~KeyFrame() {}

    size_t frame_number() const { return frame_num; }
    const Frame& get_frame() const { return frame; }
    Frame& get_frame() { return frame; }

  private:
    size_t frame_num;
    Frame frame;
};

class Animation
{
  public:
    Animation(size_t frame_count, const std::vector<KeyFrame>& key_frames)
        : number_of_frames(frame_count)
        , key_frames(key_frames)
    {
        if (key_frames.size() == 0)
            throw std::runtime_error("No keyframes given");

        auto comp = [](const KeyFrame& f1, const KeyFrame& f2) { return f1.frame_number() < f2.frame_number(); };
        std::sort(this->key_frames.begin(), this->key_frames.end(), comp);
    }
    ~Animation() {}

    const KeyFrame& key_frame(size_t i) const { return key_frames[i]; }

    size_t frame_count() const { return number_of_frames; }
    size_t key_frame_count() const { return key_frames.size(); }

    // Returns the index of the keyframe that comes before the given frame
    size_t index_of(float frame) const
    {
        for (size_t i = 0; i < key_frames.size(); i++)
            if (key_frames[i].frame_number() > frame)
                return i - 1;
        return key_frames.size() - 1;
    }

  private:
    size_t number_of_frames;
    std::vector<KeyFrame> key_frames;
};