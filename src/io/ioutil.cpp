#include <algorithm>
#include <cstring>
#include <fstream>
#include <string>

#include "ioutil.h"

std::string str_from_file(const std::string& path)
{
    std::ifstream stream(path);
    if (stream.fail())
        throw std::runtime_error("Could not open file " + path);

    return std::string((std::istreambuf_iterator<char>(stream)),
                       (std::istreambuf_iterator<char>()));
}

void filter_string(std::string& raw, const std::string& chars_to_remove)
{
    const char* rem = chars_to_remove.c_str();
    for (int i = 0; i < strlen(rem); i++) {
        raw.erase(std::remove(raw.begin(), raw.end(), rem[i]), raw.end());
    }
}

std::string directory_of(const std::string& file)
{
    size_t i = file.find_last_of("\\/");
    return std::string::npos == i ? "" : file.substr(0, i + 1);
}