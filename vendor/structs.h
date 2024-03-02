#ifndef STRUCTS_H
#define STRUCTS_H

#include <vector>

struct HeVec3f
{
    float x, y, z;
};

struct HeVertex
{
    float x, y, z;
};

struct HeFace
{
    int idx1, idx2, idx3;
};

struct HeMesh_Data
{
    std::vector<HeVertex*> vertices;
    std::vector<HeFace*> faces;
};

#endif
