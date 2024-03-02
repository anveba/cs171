#pragma once

#include <string>
#include <vector>

#include "algebra.h"

struct Vertex
{
    Vec3 position;
    Vec3 normal;
};

// A triangle struct that refers to its vertices through indices
struct IndexedTriangle
{
    uint32_t position_indices[3];
    uint32_t normal_indices[3];
};

// A triangle struct that owns its vertices
struct OwnedTriangle
{
    Vertex vertices[3];
};

// A geometric mesh represented by a list of vertices and faces which store
// three indices that refer to the vertices, much like the obj format.
// It uses 0-indexing.
class Mesh
{
  public:
    Mesh() {}

    Mesh(const std::vector<Vec3>& vertex_positions,
         const std::vector<Vec3>& vertex_normals,
         const std::vector<IndexedTriangle>& triangles)
        : vertex_positions(vertex_positions)
        , vertex_normals(vertex_normals)
        , tris(triangles)
    {
    }

    const std::vector<Vec3>& get_vertex_positions() const { return vertex_positions; }
    const std::vector<Vec3>& get_vertex_normals() const { return vertex_normals; }
    const std::vector<IndexedTriangle>& get_indexed_triangles() const { return tris; }

    void recalculate_normals();
    void implicit_fairing(float h);

    std::vector<OwnedTriangle> owned_triangles() const;
    void create_buffers(std::vector<Vec3>& positions, std::vector<Vec3>& normals) const;

  private:
    std::vector<Vec3> vertex_positions;
    std::vector<Vec3> vertex_normals;
    std::vector<IndexedTriangle> tris;
};