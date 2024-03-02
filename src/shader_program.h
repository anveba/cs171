#pragma once

#include "algebra.h"
#include "fragment_shader.h"
#include "texture2d.h"
#include "vertex_shader.h"

class ShaderProgram
{
  public:
    static ShaderProgram link(const VertexShader& vert, const FragmentShader& frag);

    void set_uniform(const std::string& name, int i) const;
    void set_uniform(const std::string& name, const Vec3& v) const;

    void use() const;
    static void unuse();

    ShaderProgram();
    ~ShaderProgram();
    ShaderProgram(ShaderProgram&&);
    ShaderProgram& operator=(ShaderProgram&&);
    ShaderProgram(ShaderProgram const&) = delete;
    void operator=(ShaderProgram const&) = delete;

  private:
    ShaderProgram(unsigned int handle);

    unsigned int handle;
};