#pragma once

#include <stdexcept>
#include <string>

class VertexShader
{
  public:
    static VertexShader from_source(const std::string& src);

    ~VertexShader();
    VertexShader(VertexShader const&) = delete;
    void operator=(VertexShader const&) = delete;
    VertexShader(VertexShader&&);

  private:
    VertexShader(unsigned int handle);

    friend class ShaderProgram;

    unsigned int handle;
};