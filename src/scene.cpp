#include "scene.h"

void MeshBuffers::update_buffers()
{
    positions.clear();
    normals.clear();
    mesh.create_buffers(positions, normals);
}