#pragma once

#include <string>

class Texture2D
{
  public:
    static Texture2D from_file(const std::string& path);
    static void bind(const Texture2D& tex);
    static void unbind();
    static void set_active(int slot);

    ~Texture2D();

    Texture2D(Texture2D const&) = delete;
    void operator=(Texture2D const&) = delete;
    Texture2D(Texture2D&&);

  private:
    Texture2D(unsigned int handle);

    unsigned int handle;
};