#include "voxel_frame_shader.h"
#include "ray.h"
#include "render_world.h"
#include "voxelized_mesh.h"

vec3 Voxel_Frame_Shader::
Shade_Surface(const Ray &ray, const vec3 &intersection_point,
              const vec3 &normal, int recursion_depth, const Hit &hit) const
{
    const Voxelized_Mesh *voxelized_mesh = dynamic_cast<const Voxelized_Mesh *>(hit.object);
    vec3 voxel_lo = voxelized_mesh->voxels[hit.part];
    double voxel_size = voxelized_mesh->voxel_size;

    vec3 distance_to_lo = intersection_point - voxel_lo;
    int close_margin_count = 0;
    for (int i = 0; i < 3; i++)
    {
        if (distance_to_lo[i] < threshold * voxel_size ||
            voxel_size - distance_to_lo[i] < threshold * voxel_size)
        {
            close_margin_count++;
        }
    }
    if (close_margin_count > 1)
    {
        return color;
    }

    vec3 real_shader_color = shader->Shade_Surface(ray, intersection_point, normal, recursion_depth, hit);

    return real_shader_color;
}
