#include "phong_shader.h"

Colour PhongShader::shade(const SurfacePoint& surface_point,
                          const PhongMaterial& material,
                          const Scene& scene) const
{
    Vec3 cam_dir = (scene.camera().position() - surface_point.world_position).normalized();

    Colour diffuse, specular;

    for (auto light : scene.get_point_lights()) {

        Vec3 light_dir = (light.position() - surface_point.world_position);
        float light_dist = light_dir.norm();
        light_dir /= light_dist;

        Colour light_col = light.colour() /
                           (1.0f + light.get_attenuation() * light_dist * light_dist);

        Vec3 unit_normal = surface_point.normal.normalized();

        diffuse += light_col * std::max(0.0f, light_dir.dot(unit_normal));

        specular += light_col *
                    std::pow(
                        std::max(0.0f, unit_normal.dot((cam_dir + light_dir).normalized())),
                        material.shininess());
    }

    // Note: Colour gets clamped later on
    return material.ambient() +
           material.diffuse() * diffuse +
           material.specular() * specular;
}