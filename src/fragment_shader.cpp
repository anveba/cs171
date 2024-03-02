#include "fragment_shader.h"

#define GL_GLEXT_PROTOTYPES 1
#include <GL/gl.h>
#include <GL/glu.h>

#include <iostream>

FragmentShader FragmentShader::from_source(const std::string& src)
{
    FragmentShader shader = FragmentShader(glCreateShader(GL_FRAGMENT_SHADER));

    auto c_src = src.c_str();
    glShaderSource(shader.handle, 1, &c_src, NULL);
    glCompileShader(shader.handle);

    GLint compile_status;
    glGetShaderiv(shader.handle, GL_COMPILE_STATUS, &compile_status);
    if (!compile_status) {

        GLint length;
        glGetShaderiv(shader.handle, GL_INFO_LOG_LENGTH, &length);

        char* error_str = new char[length];
        glGetShaderInfoLog(shader.handle, length, &length, error_str);

        std::cerr << error_str << std::endl;

        delete error_str;

        throw std::runtime_error("Could not compile fragment shader");
    }

    return shader;
}

FragmentShader::FragmentShader(unsigned int handle)
    : handle(handle)
{
}

FragmentShader::~FragmentShader()
{
    glDeleteShader(handle);
}

FragmentShader::FragmentShader(FragmentShader&& other)
{
    handle = other.handle;
    other.handle = 0;
}