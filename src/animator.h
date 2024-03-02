#pragma once

#include "animation.h"

class Animator
{
  public:
    Animator(const Animation& anim, bool loop);
    Animator();
    ~Animator();

    Frame interpolate_frame(float u) const;
    const Animation& get_animation() const { return animation; }
    bool& loops() { return loop; }

  private:
    Animation animation;
    bool loop;
};