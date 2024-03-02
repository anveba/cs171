#pragma once

#include "scene.h"
#include "shader_program.h"

class OpenGlRenderer
{
  public:
    void init_settings();
    void clear();
    void render(const Scene& scene, const ShaderProgram& shader);

  private:
};