#include <iostream>

#include "depth_buffer.h"
#include "triangle_renderer.h"

static bool in_unit_cube(const Vec3& pos)
{
    return pos.x() >= -1.0f && pos.x() <= 1.0f &&
           pos.y() >= -1.0f && pos.y() <= 1.0f &&
           pos.z() >= -1.0f && pos.z() <= 1.0f;
}

static bool is_back_face(Vec3 ndc_positions[3])
{
    Vec3 ab = ndc_positions[1] - ndc_positions[0];
    Vec3 ac = ndc_positions[2] - ndc_positions[0];
    return (ab.x() * ac.y() - ac.x() * ab.y()) < 0.0f;
}

// Corresponding to the function f_ij given in the lecture notes
static int f_ij(Point2 i, Point2 j, int x, int y)
{
    return (i.y() - j.y()) * x + (j.x() - i.x()) * y + i.x() * j.y() - j.x() * i.y();
}

static SurfacePoint interpolate_surface_point(float alpha, float beta, float gamma, const SurfacePoint p[3])
{
    return SurfacePoint(alpha * p[0].world_position +
                            beta * p[1].world_position +
                            gamma * p[2].world_position,
                        alpha * p[0].normal +
                            beta * p[1].normal +
                            gamma * p[2].normal

    );
}

static Vec3 interpolate_vector(float alpha, float beta, float gamma, const Vec3 vec[3])
{
    return alpha * vec[0] +
           beta * vec[1] +
           gamma * vec[2];
}

static Colour interpolate_colour(float alpha, float beta, float gamma, const Colour col[3])
{
    return alpha * col[0] +
           beta * col[1] +
           gamma * col[2];
}

// Returns whether the barycentric coordinates represent a point inside the triangle.
static bool is_in_triangle(float alpha, float beta, float gamma)
{
    return alpha >= 0.0f && alpha <= 1.0f &&
           beta >= 0.0f && beta <= 1.0f &&
           gamma >= 0.0f && gamma <= 1.0f;
}

static void rasterise_triangle(const Vec3 ndc_positions[3],
                               const SurfacePoint vertices[3],
                               const PhongMaterial& material,
                               const Scene& scene,
                               SoftwareShader* shader,
                               Image& image,
                               DepthBuffer& depth_buffer)
{
    // Shade triangle before rasterisation if we are doing per vertex shading
    Colour shaded_vertex_colours[3];
    if (!shader->per_pixel_shading())
        for (int i = 0; i < 3; i++)
            shaded_vertex_colours[i] = shader->shade(vertices[i], material, scene);

    // Get vertex positions on raster
    Point2 raster_pos[3] = {
        ndc_to_raster(ndc_positions[0], image.width(), image.height()),
        ndc_to_raster(ndc_positions[1], image.width(), image.height()),
        ndc_to_raster(ndc_positions[2], image.width(), image.height())
    };

    int x0 = std::min(raster_pos[0].x(), std::min(raster_pos[1].x(), raster_pos[2].x()));
    int x1 = std::max(raster_pos[0].x(), std::max(raster_pos[1].x(), raster_pos[2].x()));
    int y0 = std::min(raster_pos[0].y(), std::min(raster_pos[1].y(), raster_pos[2].y()));
    int y1 = std::max(raster_pos[0].y(), std::max(raster_pos[1].y(), raster_pos[2].y()));

    for (int i = std::max(0, x0); i < std::min(image.width(), x1); i++) {
        for (int j = std::max(0, y0); j < std::min(image.height(), y1); j++) {

            // Calculate barycentric coordinates
            float alpha = (float)f_ij(raster_pos[1], raster_pos[2], i, j) /
                          f_ij(raster_pos[1], raster_pos[2], raster_pos[0].x(), raster_pos[0].y());

            float beta = (float)f_ij(raster_pos[0], raster_pos[2], i, j) /
                         f_ij(raster_pos[0], raster_pos[2], raster_pos[1].x(), raster_pos[1].y());

            float gamma = (float)f_ij(raster_pos[0], raster_pos[1], i, j) /
                          f_ij(raster_pos[0], raster_pos[1], raster_pos[2].x(), raster_pos[2].y());

            if (is_in_triangle(alpha, beta, gamma)) {

                // Calculate pixel NDC coordinate
                Vec3 interpolated_ndc = interpolate_vector(alpha, beta, gamma, ndc_positions);

                // Cull pixels out of view. Note: backface culling is done earlier.
                if (depth_buffer.get_unchecked(i, j) >= interpolated_ndc.z() &&
                    in_unit_cube(interpolated_ndc)) {

                    // Shade triangle while rasterising if we are doing per pixel-shading. If
                    // we are doing per-vertex shading, interpolate between the vertex colours.
                    Colour c;
                    if (shader->per_pixel_shading()) {
                        c = shader->shade(
                            SurfacePoint(
                                interpolate_surface_point(alpha, beta, gamma, vertices)),
                            material,
                            scene);
                    } else {
                        c = interpolate_colour(alpha, beta, gamma, shaded_vertex_colours);
                    }

                    // Update the raster and depth buffer
                    assert(image.is_inside(i, j));
                    image.get_unchecked(i, j) = c;
                    depth_buffer.get_unchecked(i, j) = interpolated_ndc.z();
                }
            }
        }
    }
}

void TriangleRenderer::render(SoftwareShader* shader, Image& image, const Scene& scene)
{
    DepthBuffer depth_buffer(image.width(), image.height());
    depth_buffer.clear(std::numeric_limits<float>::infinity());

    const Mat4 world_to_ndc = scene.camera().world_to_ndc_matrix();

    // First, we get all triangles in the scene. Since vertices are referenced,
    // we have to dereference them. Then, we tranform the vertices' position along
    // with normals. Lastly, we start the rasterisation procedure for the triangle.

    for (auto instance : scene.get_instances()) {

        auto mesh = scene.get_meshes()[instance.mesh_index];

        Mat4 model_to_world = scene.global_transform().matrix() * instance.transform.matrix();

        // Calculate matrix that properly transforms normals
        Mat3 normal_mat;
        normal_mat << model_to_world(0, 0), model_to_world(0, 1), model_to_world(0, 2),
            model_to_world(1, 0), model_to_world(1, 1), model_to_world(1, 2),
            model_to_world(2, 0), model_to_world(2, 1), model_to_world(2, 2);
        normal_mat = normal_mat.inverse().transpose();

        for (int tri = 0; tri < mesh.get_positions().size(); tri += 3) {

            Vec3 ndc_positions[3];
            Vec3 world_positions[3];
            Vec3 normals[3];

            for (int i = 0; i < 3; i++) {

                // Transform position
                world_positions[i] = mesh.get_positions()[tri + i];
                Vec4 vec4_world_pos = Vec4(world_positions[i].x(),
                                           world_positions[i].y(),
                                           world_positions[i].z(),
                                           1.0f);

                vec4_world_pos = model_to_world * vec4_world_pos;
                world_positions[i] = Vec3(vec4_world_pos.x(), vec4_world_pos.y(), vec4_world_pos.z());
                Vec4 ndc_homog_pos = world_to_ndc * vec4_world_pos;

                ndc_positions[i] = Vec3(
                    ndc_homog_pos.x() / ndc_homog_pos.w(),
                    ndc_homog_pos.y() / ndc_homog_pos.w(),
                    ndc_homog_pos.z() / ndc_homog_pos.w());

                // Transform normal
                normals[i] = normal_mat * mesh.get_normals()[tri + i];
            }

            // Rasterise the triangle
            if (!is_back_face(ndc_positions)) {

                SurfacePoint surface_points[3] = {
                    SurfacePoint(world_positions[0], normals[0]),
                    SurfacePoint(world_positions[1], normals[1]),
                    SurfacePoint(world_positions[2], normals[2])
                };

                rasterise_triangle(ndc_positions,
                                   surface_points,
                                   instance.material,
                                   scene,
                                   shader,
                                   image,
                                   depth_buffer);
            }
        }
    }
}