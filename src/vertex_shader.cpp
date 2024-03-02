#include "vertex_shader.h"

#define GL_GLEXT_PROTOTYPES 1
#include <GL/gl.h>
#include <GL/glu.h>

#include <iostream>

VertexShader VertexShader::from_source(const std::string& src)
{
    VertexShader shader = VertexShader(glCreateShader(GL_VERTEX_SHADER));

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

        throw std::runtime_error("Could not compile vertex shader");
    }

    return shader;
}

VertexShader::VertexShader(unsigned int handle)
    : handle(handle)
{
}

VertexShader::~VertexShader()
{
    glDeleteShader(handle);
}

VertexShader::VertexShader(VertexShader&& other)
{
    handle = other.handle;
    other.handle = 0;
}