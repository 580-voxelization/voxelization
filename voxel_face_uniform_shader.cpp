#include "voxel_face_uniform_shader.h"
#include "ray.h"
#include "render_world.h"
#include "voxelized_mesh.h"

const static vec3 normal_order[] = {
        vec3(0, 0, 1),
        vec3(0, 0, -1),
        vec3(0, 1, 0),
        vec3(0, -1, 0),
        vec3(1, 0, 0),
        vec3(-1, 0, 0),
};

const static double tol = 1e-4;

int Calculate_Face_Index(const vec3 &normal)
{
    for (int i = 0; i < 6; i++)
    {
        const vec3 &n = normal_order[i];
        if ((normal - n).magnitude_squared() < tol)
        {
            return i;
        }
    }
    return -1;
}

vec3 Voxel_Face_Uniform_Shader::
Shade_Surface(const Ray &ray, const vec3 &intersection_point,
              const vec3 &normal, int recursion_depth, const Hit &hit) const
{
    const Voxelized_Mesh *voxelized_mesh = dynamic_cast<const Voxelized_Mesh *>(hit.object);
    if (color_caches.count(voxelized_mesh) == 0)
    {
        color_caches[voxelized_mesh] = std::vector<std::vector<vec3>>();
        color_cache_valid[voxelized_mesh] = std::vector<std::vector<bool>>();
        color_caches[voxelized_mesh].resize(voxelized_mesh->voxels.size());
        color_cache_valid[voxelized_mesh].resize(voxelized_mesh->voxels.size());
    }
    if (color_cache_valid[voxelized_mesh][hit.part].empty())
    {
        color_caches[voxelized_mesh][hit.part].resize(6);
        color_cache_valid[voxelized_mesh][hit.part].resize(6);
    }

    int face_index = Calculate_Face_Index(normal);

    if (color_cache_valid[voxelized_mesh][hit.part][face_index])
    {
        return color_caches[voxelized_mesh][hit.part][face_index];
    }

    vec3 center = voxelized_mesh->voxels[hit.part] +
                  vec3(voxelized_mesh->voxel_size, voxelized_mesh->voxel_size, voxelized_mesh->voxel_size) / 2;

    vec3 modified_point = center + normal * (voxelized_mesh->voxel_size / 2);

    Ray modified_ray = ray;
    modified_ray.direction = (modified_point - ray.endpoint).normalized();

    vec3 color = shader->Shade_Surface(modified_ray, modified_point, normal, recursion_depth, hit);

    color_caches[voxelized_mesh][hit.part][face_index] = color;
    color_cache_valid[voxelized_mesh][hit.part][face_index] = true;
    return color;
}
