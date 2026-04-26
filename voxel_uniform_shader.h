#ifndef __VOXEL_UNIFORM_SHADER_H__
#define __VOXEL_UNIFORM_SHADER_H__

#include <algorithm>
#include "shader.h"

class Voxel_Uniform_Shader : public Shader
{
public:
    Shader *shader;

    Voxel_Uniform_Shader(Render_World &world_input, Shader *shader_input)
            : Shader(world_input), shader(shader_input) {}

    virtual vec3 Shade_Surface(const Ray &ray, const vec3 &intersection_point,
                               const vec3 &normal, int recursion_depth, const Hit &hit) const override;
};

#endif
