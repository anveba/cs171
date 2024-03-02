#pragma once

#include <vector>

#include "camera.h"
#include "light.h"
#include "material.h"
#include "mesh.h"
#include "transform.h"

// A 'copy' of an object (by reference through an index) with
// an associated transformation and material.
struct Instance
{
    Instance(size_t mesh_index,
             const Transform& transform,
             const PhongMaterial& material)
        : mesh_index(mesh_index)
        , transform(transform)
        , material(material)
    {
    }

    size_t mesh_index;
    Transform transform;
    PhongMaterial material;
};

class MeshBuffers
{
  public:
    MeshBuffers(const std::string& identifier, const Mesh& mesh)
        : identifier(identifier)
        , mesh(mesh)
    {
        update_buffers();
    }

    const std::string& get_identifier() const { return identifier; }
    const std::vector<Vec3>& get_positions() const { return positions; }
    const std::vector<Vec3>& get_normals() const { return normals; }
    Mesh& get_mesh() { return mesh; }
    const Mesh& get_mesh() const { return mesh; }
    void update_buffers();

  private:
    std::string identifier;
    std::vector<Vec3> positions;
    std::vector<Vec3> normals;
    Mesh mesh;
};

// A collection of instances of objects along with a camera. Also owns the meshes that
// the instances refer to.
class Scene
{
  public:
    Scene()
        : cam(Camera(Mat4::Identity(), Mat4::Identity(), Mat4::Identity()))
        , transform(Mat4::Identity())
    {
    }

    Scene(const std::vector<MeshBuffers>& meshes,
          const std::vector<Instance>& instances,
          const std::vector<PointLight>& point_lights,
          const Camera& camera)
        : meshes(meshes)
        , instances(instances)
        , point_lights(point_lights)
        , cam(camera)
        , transform(Mat4::Identity())
    {
    }

    std::vector<MeshBuffers>& get_meshes() { return meshes; }
    const std::vector<MeshBuffers>& get_meshes() const { return meshes; }
    std::vector<Instance>& get_instances() { return instances; }
    const std::vector<Instance>& get_instances() const { return instances; }
    const std::vector<PointLight>& get_point_lights() const { return point_lights; }
    Camera& camera() { return cam; }
    const Camera& camera() const { return cam; }
    Transform& global_transform() { return transform; }
    const Transform& global_transform() const { return transform; }

  private:
    std::vector<MeshBuffers> meshes;
    std::vector<Instance> instances;

    std::vector<PointLight> point_lights;

    Camera cam;

    Transform transform;
};