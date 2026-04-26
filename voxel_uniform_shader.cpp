#include "voxel_uniform_shader.h"
#include "ray.h"
#include "render_world.h"
#include "voxelized_mesh.h"

vec3 Voxel_Uniform_Shader::
Shade_Surface(const Ray &ray, const vec3 &intersection_point,
              const vec3 &normal, int recursion_depth, const Hit &hit) const
{
    const Voxelized_Mesh *voxelized_mesh = dynamic_cast<const Voxelized_Mesh *>(hit.object);
    if (color_caches.count(voxelized_mesh) == 0)
    {
        color_caches[voxelized_mesh] = std::vector<vec3>();
        color_cache_valid[voxelized_mesh] = std::vector<bool>();
        color_caches[voxelized_mesh].resize(voxelized_mesh->voxels.size());
        color_cache_valid[voxelized_mesh].resize(voxelized_mesh->voxels.size());
    }
    if (color_cache_valid[voxelized_mesh][hit.part])
    {
        return color_caches[voxelized_mesh][hit.part];
    }

    std::vector<int> triangle_indices = voxelized_mesh->voxels_to_triangles[hit.part];
    vec3 center = voxelized_mesh->voxels[hit.part] +
                  vec3(voxelized_mesh->voxel_size, voxelized_mesh->voxel_size, voxelized_mesh->voxel_size) / 2;

    int closest_triangle_index = -1;
    double min_distance;
    vec3 closest_mesh_point;
    for (int index: triangle_indices)
    {
        vec3 target_point;
        double distance = voxelized_mesh->Distance_To_Triangle(center, index, target_point);
        if (closest_triangle_index == -1 || distance < min_distance)
        {
            closest_triangle_index = index;
            min_distance = distance;
            closest_mesh_point = target_point;
        }
    }

    vec3 mesh_normal = voxelized_mesh->mesh.Normal(closest_mesh_point, closest_triangle_index);

    Ray modified_ray = ray;
    modified_ray.direction = (closest_mesh_point - ray.endpoint).normalized();

    vec3 color = shader->Shade_Surface(modified_ray, closest_mesh_point, mesh_normal, recursion_depth, hit);

    color_caches[voxelized_mesh][hit.part] = color;
    color_cache_valid[voxelized_mesh][hit.part] = true;
    return color;
}
