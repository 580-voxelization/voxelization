#ifndef __VOXEL_FRAME_SHADER_H__
#define __VOXEL_FRAME_SHADER_H__

#include <algorithm>
#include <map>
#include "shader.h"
#include "voxelized_mesh.h"

class Voxel_Frame_Shader : public Shader
{
public:
    Shader *shader;
    double threshold;
    vec3 color;

    Voxel_Frame_Shader(Render_World &world_input, double threshold, vec3 color, Shader *shader_input)
            : Shader(world_input), threshold(threshold), color(color), shader(shader_input) {}

    virtual vec3 Shade_Surface(const Ray &ray, const vec3 &intersection_point,
                               const vec3 &normal, int recursion_depth, const Hit &hit) const override;
};

#endif
