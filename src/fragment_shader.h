#pragma once

#include <stdexcept>
#include <string>

class FragmentShader
{
  public:
    static FragmentShader from_source(const std::string& src);

    ~FragmentShader();
    FragmentShader(FragmentShader const&) = delete;
    void operator=(FragmentShader const&) = delete;
    FragmentShader(FragmentShader&&);

  private:
    FragmentShader(unsigned int handle);

    friend class ShaderProgram;

    unsigned int handle;
};