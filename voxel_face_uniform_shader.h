#ifndef __VOXEL_FACE_UNIFORM_SHADER_H__
#define __VOXEL_FACE_UNIFORM_SHADER_H__

#include <algorithm>
#include <map>
#include "shader.h"
#include "voxelized_mesh.h"

class Voxel_Face_Uniform_Shader : public Shader
{
public:
    Shader *shader;
    mutable std::map<const Voxelized_Mesh *, std::vector<std::vector<vec3>>> color_caches;
    mutable std::map<const Voxelized_Mesh *, std::vector<std::vector<bool>>> color_cache_valid;

    Voxel_Face_Uniform_Shader(Render_World &world_input, Shader *shader_input)
            : Shader(world_input), shader(shader_input) {}

    virtual vec3 Shade_Surface(const Ray &ray, const vec3 &intersection_point,
                               const vec3 &normal, int recursion_depth, const Hit &hit) const override;
};

#endif
