#include "shader_program.h"

#define GL_GLEXT_PROTOTYPES 1
#include <GL/gl.h>
#include <GL/glu.h>
#include <stdexcept>

ShaderProgram ShaderProgram::link(const VertexShader& vert, const FragmentShader& frag)
{
    ShaderProgram program = ShaderProgram(glCreateProgram());

    glAttachShader(program.handle, vert.handle);
    glAttachShader(program.handle, frag.handle);
    glLinkProgram(program.handle);

    GLint link_status;
    glGetProgramiv(program.handle, GL_LINK_STATUS, &link_status);
    if (!link_status) {
        throw std::runtime_error("Could not link shader");
    }

    return program;
}

void ShaderProgram::set_uniform(const std::string& name, int i) const
{
    auto loc = glGetUniformLocation(handle, name.c_str());
    glUniform1i(loc, i);
}

void ShaderProgram::set_uniform(const std::string& name, const Vec3& v) const
{
    auto loc = glGetUniformLocation(handle, name.c_str());
    glUniform3f(loc, v.x(), v.y(), v.z());
}

void ShaderProgram::use() const
{
    glUseProgram(handle);
}

void ShaderProgram::unuse()
{
    glUseProgram(0);
}

ShaderProgram::ShaderProgram()
    : handle(0)
{
}

ShaderProgram::ShaderProgram(unsigned int handle)
    : handle(handle)
{
}

ShaderProgram::~ShaderProgram()
{
    glDeleteProgram(handle);
}

ShaderProgram::ShaderProgram(ShaderProgram&& other)
{
    handle = other.handle;
    other.handle = 0;
}

ShaderProgram& ShaderProgram::operator=(ShaderProgram&& other)
{
    if (&other == this)
        return *this;

    glDeleteProgram(handle);

    handle = other.handle;
    other.handle = 0;

    return *this;
}