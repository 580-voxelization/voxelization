#ifndef __RANDOM_FLAT_SHADER_H__
#define __RANDOM_FLAT_SHADER_H__

#include "shader.h"
#include <random>

class Random_Shader : public Shader
{
    mutable std::mt19937 gen;
    mutable std::uniform_real_distribution<double> dis;

public:
    Random_Shader(Render_World &world_input)
            : Shader(world_input), gen(std::random_device()()), dis(0.0, 1.0)
    {
    }

    virtual vec3 Shade_Surface(const Ray &ray, const vec3 &intersection_point,
                               const vec3 &normal, int recursion_depth) const override;
};

#endif
