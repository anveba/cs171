#include <assert.h>

#include "wireframe_renderer.h"

WireframeRenderer::WireframeRenderer()
{
}

// Uses Bresenham’s line algorithm.
//
// Generalisation:
// The ungeneralised version only works when x0 < x1 and the slope is between 0 and 1.
// In order to generalise it, we need to convert all other scenarios into one where
// x0 < x1 and the slope is between 0 and 1 and compensate for the conversion.
//
// We will first look at lines with steep and positive slopes, that is, lines a
// slope between 1 and +infinity. In this case, we can swap the components of the
// coordinates (that is, swap x and y) to mirror it w.r.t. the line y = x. Now, the
// slope is between 0 and 1, as desired. However, when actually drawing pixels, we
// need to remember to 'unswap' x and y.
//
// When the slope is negative, we can decrement y (instead of incrementing) when the
// inequality 2 * (epsilon_prime + dy) < dx doesn't hold. We also flip the sign of dy.
// With this, the program can continue as is the slope were positive. The dir_y variable
// keeps track of the sign of the slope.
//
// When x0 > x1, we can compensate by swapping the points. This results in the same line.
//
// Combining these, we can draw lines for any given two points.
static void draw_line(int x0, int y0, int x1, int y1, Image& image)
{
    // If the slope is steep, we swap x and y and remember we have done so.
    bool components_swapped = false;
    if (fabsf(y1 - y0) > fabsf(x1 - x0)) {
        std::swap(x0, y0);
        std::swap(x1, y1);
        components_swapped = true;
    }

    // We always want x0 < x1
    if (x0 > x1) {
        std::swap(x0, x1);
        std::swap(y0, y1);
    }

    int dx = x1 - x0;
    int dy = y1 - y0;

    int dir_y = dy < 0 ? -1 : 1;
    dy *= dir_y;

    int epsilon_prime = 0;
    int y = y0;

    assert(x0 <= x1);
    assert(dy >= 0);
    assert(dx >= 0);
    assert(dx >= dy);

    // *Very* quick and dirty anti-aliasing; it is only controlled by this bool as is
    // therefore baked into the program at compile-time.
    bool antialiasing = true;

    for (int x = x0; x <= x1; x++) {

        int draw_x = x, draw_y = y;
        if (components_swapped) //'Unswap' components if they were initially swapped
            std::swap(draw_x, draw_y);

        if (antialiasing) {
            float f = (float)epsilon_prime / dx + 0.5f;
            if (image.is_inside(draw_x, draw_y))
                image.get_unchecked(draw_x, draw_y) += Colour(1.0f - f);

            draw_x = draw_x + components_swapped * dir_y;
            draw_y = draw_y + !components_swapped * dir_y;

            if (image.is_inside(draw_x, draw_y))
                image.get_unchecked(draw_x, draw_y) += Colour(f);

        } else {
            if (image.is_inside(draw_x, draw_y))
                image.get_unchecked(draw_x, draw_y) = Colour(1.0f);
        }

        if (2 * (epsilon_prime + dy) < dx) {
            epsilon_prime += dy;
        } else {
            epsilon_prime += dy - dx;
            y += dir_y;
        }
    }
}

static void draw_triangle_frame(const Vec3 tri[3], Image& image)
{
    for (int i = 0; i < 3; i++) {
        Point2 p1 = ndc_to_raster(tri[i],
                                  image.width(),
                                  image.height());
        Point2 p2 = ndc_to_raster(tri[(i + 1) % 3],
                                  image.width(),
                                  image.height());
        draw_line(p1.x(), p1.y(), p2.x(), p2.y(), image);
    }
}

static bool in_unit_square(const Vec3& pos)
{
    return pos.x() >= -1.0f && pos.x() <= 1.0f &&
           pos.y() >= -1.0f && pos.y() <= 1.0f;
}

void WireframeRenderer::render(Image& image, const Scene& scene)
{
    const Mat4 world_to_ndc = scene.camera().world_to_ndc_matrix();

    for (auto instance : scene.get_instances()) {

        auto mesh = scene.get_meshes()[instance.mesh_index];
        const auto& model_to_world = scene.global_transform().matrix() * instance.transform.matrix();

        for (int tri = 0; tri < mesh.get_positions().size(); tri += 3) {

            // For each triangle in every instance, we transform its vertices to world
            // coordinates and then convert them to cartesian NDC coordinate, where we
            // include it in the output if it is inside the unit cube.

            bool is_in_frustum = false;

            Vec3 ndc_positions[3];

            for (int i = 0; i < 3; i++) {

                Vec3& pos = ndc_positions[i];
                pos = mesh.get_positions()[tri + i];

                Vec4 vec4_pos = Vec4(pos.x(), pos.y(), pos.z(), 1.0f);
                Vec4 ndc_pos = world_to_ndc * model_to_world * vec4_pos;

                pos = Vec3(
                    ndc_pos.x() / ndc_pos.w(),
                    ndc_pos.y() / ndc_pos.w(),
                    ndc_pos.z() / ndc_pos.w());

                // Apparently we don't have to check the z coordinate for this assignment.
                if (in_unit_square(pos))
                    is_in_frustum = true;
            }

            if (is_in_frustum)
                draw_triangle_frame(ndc_positions, image);
        }
    }
}
