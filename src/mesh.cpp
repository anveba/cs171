#include "mesh.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wignored-attributes"
#include <Eigen/Dense>
#include <Eigen/Sparse>
#pragma GCC diagnostic pop

#include <halfedge.h>
#include <structs.h>

// Converts the Mesh class into the halfedge compatible struct
static HeMesh_Data to_mesh_data(const Mesh& mesh)
{
    HeMesh_Data mesh_data;
    auto& he_vertices = mesh_data.vertices;
    he_vertices.reserve(mesh.get_vertex_positions().size());
    for (int i = 0; i < mesh.get_vertex_positions().size(); i++)
        he_vertices.push_back((HeVertex*)&mesh.get_vertex_positions()[i]);

    auto& he_faces = mesh_data.faces;
    he_faces.reserve(mesh.get_indexed_triangles().size());
    for (int i = 0; i < mesh.get_indexed_triangles().size(); i++)
        he_faces.push_back((HeFace*)&mesh.get_indexed_triangles()[i]);

    return mesh_data;
}

// Calculates new normals based on the area-weighted normal approach
void Mesh::recalculate_normals()
{
    std::vector<HEV*> hevs;
    std::vector<HEF*> hefs;
    auto mesh_data = to_mesh_data(*this);

    build_HE(&mesh_data, &hevs, &hefs);

    vertex_normals.clear();
    vertex_normals.reserve(vertex_positions.size());

    // For each vertex in each triangle, we calculate the new normal
    for (int i = 0; i < tris.size(); i++) {
        for (int j = 0; j < 3; j++) {

            const auto& v = hevs[tris[i].position_indices[j]];
            Vec3 normal = Vec3::Zero();

            // Traverse the neighboring vertices using the halfedge data structure
            HE* he = v->out;
            do {
                HEF* f = he->face;

                HEV* v1 = f->edge->vertex;
                HEV* v2 = f->edge->next->vertex;
                HEV* v3 = f->edge->next->next->vertex;

                Vec3 face_normal = Vec3(v2->x - v1->x, v2->y - v1->y, v2->z - v1->z)
                                       .cross(Vec3(v3->x - v1->x, v3->y - v1->y, v3->z - v1->z));
                float area = 0.5f * face_normal.norm();
                normal += face_normal * area;

                he = he->flip->next;
            } while (he != v->out);

            // Set the new normal
            tris[i].normal_indices[j] = vertex_normals.size();
            vertex_normals.push_back(normal.normalized());
        }
    }
}

// Calculates 1/(2A) for each vertex based on the surrounding triangles. The given
// float buffer will be populated with the values with the value at index i corresponding
// to the value of vertex i.
static void get_half_inverse_areas(const std::vector<HEV*>& hevs, float* half_inv_areas)
{
    for (const auto& v_i : hevs) {
        half_inv_areas[v_i->index] = 0.0f;
        HE* he = v_i->out;

        do {
            HEF* f = he->face;

            HEV* v1 = f->edge->vertex;
            HEV* v2 = f->edge->next->vertex;
            HEV* v3 = f->edge->next->next->vertex;

            Vec3 face_normal = Vec3(v2->x - v1->x, v2->y - v1->y, v2->z - v1->z)
                                   .cross(Vec3(v3->x - v1->x, v3->y - v1->y, v3->z - v1->z));
            half_inv_areas[v_i->index] += 0.5f * face_normal.norm();

            he = he->flip->next;
        } while (he != v_i->out);

        if (half_inv_areas[v_i->index] < 1e-5)
            half_inv_areas[v_i->index] = 0.0f;
        else
            half_inv_areas[v_i->index] = 1.0f / (2.0f * half_inv_areas[v_i->index]);
    }
}

// Builds the coefficient matrix corresponding to the discrete laplacian for all vertices
static Eigen::SparseMatrix<float> build_discrete_laplacian(
    const std::vector<HEV*>& hevs,
    float* half_inv_areas)
{
    Eigen::SparseMatrix<float> delta(hevs.size(), hevs.size());
    delta.reserve(Eigen::VectorXi::Constant(hevs.size(), 7));

    for (const auto& v_i : hevs) {

        delta.insert(v_i->index, v_i->index) = 0;

        // Traverse each surrounding vertex of v_i and find cot(a_j) and cot(b_j) to insert
        // into the matrix.
        HE* he = v_i->out;
        do {
            HEV* v_j = he->next->vertex;

            Vec3 v_i_pos = Vec3(v_i->x, v_i->y, v_i->z);
            Vec3 v_j_pos = Vec3(v_j->x, v_j->y, v_j->z);

            // Calculate cot(a_j)
            float cot_a;
            {
                HEV* v_a = he->flip->next->next->vertex;
                Vec3 v_a_pos = Vec3(v_a->x, v_a->y, v_a->z);
                Vec3 a1_vec = v_i_pos - v_a_pos;
                Vec3 a2_vec = v_j_pos - v_a_pos;
                cot_a = a1_vec.dot(a2_vec) / a1_vec.cross(a2_vec).norm();
            }

            // Calculate cot(b_j)
            float cot_b;
            {
                HEV* v_b = he->next->next->vertex;
                Vec3 v_b_pos = Vec3(v_b->x, v_b->y, v_b->z);
                Vec3 b1_vec = v_i_pos - v_b_pos;
                Vec3 b2_vec = v_j_pos - v_b_pos;
                cot_b = b1_vec.dot(b2_vec) / b1_vec.cross(b2_vec).norm();
            }

            // Update values in matrix
            delta.coeffRef(v_i->index, v_i->index) += (-cot_a - cot_b) * half_inv_areas[v_i->index];
            delta.insert(v_i->index, v_j->index) = (cot_a + cot_b) * half_inv_areas[v_i->index];

            he = he->flip->next;
        } while (he != v_i->out);
    }

    return delta;
}

void Mesh::implicit_fairing(float h)
{
    std::vector<HEV*> hevs;
    std::vector<HEF*> hefs;
    auto mesh_data = to_mesh_data(*this);

    build_HE(&mesh_data, &hevs, &hefs);

    // Set the indices for the vertices in the halfedge data structure
    for (int i = 0; i < hevs.size(); i++)
        hevs[i]->index = i;

    float* half_inv_areas = new float[hevs.size()];

    get_half_inverse_areas(hevs, half_inv_areas);

    auto delta = build_discrete_laplacian(hevs, half_inv_areas);

    delete[] half_inv_areas;

    Eigen::SparseMatrix<float> ident(hevs.size(), hevs.size());
    ident.setIdentity();

    auto f = ident - h * delta;

    // op.makeCompressed();

    Eigen::SparseLU<Eigen::SparseMatrix<float>> solver;

    solver.analyzePattern(f);
    solver.factorize(f);

    // Create right hand side of the equation
    Eigen::VectorXf x_0(hevs.size()), y_0(hevs.size()), z_0(hevs.size());
    for (int i = 0; i < hevs.size(); i++) {
        x_0[i] = vertex_positions[i].x();
        y_0[i] = vertex_positions[i].y();
        z_0[i] = vertex_positions[i].z();
    }

    // Solve
    Eigen::VectorXf x_h(hevs.size()), y_h(hevs.size()), z_h(hevs.size());
    x_h = solver.solve(x_0);
    y_h = solver.solve(y_0);
    z_h = solver.solve(z_0);

    // Update vertex positions
    for (int i = 0; i < vertex_positions.size(); i++) {
        auto& v = vertex_positions[i];
        v.x() = x_h[i];
        v.y() = y_h[i];
        v.z() = z_h[i];
    }

    recalculate_normals();
}

std::vector<OwnedTriangle> Mesh::owned_triangles() const
{
    std::vector<OwnedTriangle> res;

    for (auto tri : tris) {
        OwnedTriangle owned_tri;
        for (int i = 0; i < 3; i++) {
            owned_tri.vertices[i].position = vertex_positions[tri.position_indices[i]];
            owned_tri.vertices[i].normal = vertex_normals[tri.normal_indices[i]];
        }
        res.push_back(owned_tri);
    }
    return res;
}

void Mesh::create_buffers(std::vector<Vec3>& positions, std::vector<Vec3>& normals) const
{
    for (auto tri : tris) {
        for (int i = 0; i < 3; i++) {
            positions.push_back(vertex_positions[tri.position_indices[i]]);
            normals.push_back(vertex_normals[tri.normal_indices[i]]);
        }
    }
}
