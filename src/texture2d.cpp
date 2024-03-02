#include "texture2d.h"

#define GL_GLEXT_PROTOTYPES 1
#include <GL/gl.h>
#include <GL/glu.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

Texture2D Texture2D::from_file(const std::string& path)
{
    stbi_set_flip_vertically_on_load(true);
    int width, height, comp;
    unsigned char* data = stbi_load(path.c_str(), &width, &height, &comp, 4);

    unsigned int handle;
    glGenTextures(1, &handle);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, handle);
    glTexImage2D(GL_TEXTURE_2D, 0, comp, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);

    stbi_image_free(data);

    return Texture2D(handle);
}

void Texture2D::bind(const Texture2D& tex)
{
    glBindTexture(GL_TEXTURE_2D, tex.handle);
}

void Texture2D::unbind()
{
    glBindTexture(GL_TEXTURE_2D, 0);
}

void Texture2D::set_active(int slot)
{
    assert(slot >= 0);
    glActiveTexture(GL_TEXTURE0 + slot);
}

Texture2D::Texture2D(unsigned int handle)
    : handle(handle)
{
}

Texture2D::~Texture2D()
{
    if (handle)
        glDeleteTextures(1, &handle);
}

Texture2D::Texture2D(Texture2D&& other)
{
    handle = other.handle;
    other.handle = 0;
}