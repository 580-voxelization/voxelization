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

    vec3 accumulated_color;
    double accumulated_weight;
    for (int index: triangle_indices)
    {
        vec3 target_point;
        double distance = voxelized_mesh->Distance_To_Triangle(center, index, target_point);
        double weight = 1 / distance * distance;

        vec3 mesh_normal = voxelized_mesh->mesh.Normal(target_point, index);

        Ray modified_ray = ray;
        modified_ray.direction = (target_point - ray.endpoint).normalized();

        vec3 color = shader->Shade_Surface(modified_ray, target_point, mesh_normal, recursion_depth, hit);
        accumulated_color += color * weight;
        accumulated_weight += weight;
    }

    vec3 color = accumulated_color /= accumulated_weight;

    color_caches[voxelized_mesh][hit.part] = color;
    color_cache_valid[voxelized_mesh][hit.part] = true;
    return color;
}
