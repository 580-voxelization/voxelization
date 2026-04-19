#ifndef __RANDOM_FLAT_SHADER_H__
#define __RANDOM_FLAT_SHADER_H__

#include "shader.h"
#include <random>

class Random_Shader : public Shader
{
    const int size;

    mutable std::mt19937 gen;
    mutable std::uniform_real_distribution<double> dis;
    std::vector<vec3> colors;

public:
    Random_Shader(Render_World &world_input, int size)
            : Shader(world_input), size(size), gen(std::random_device()()), dis(0.0, 1.0)
    {
        for (int i = 0; i < size; i++)
        {
            colors.emplace_back(dis(gen), dis(gen), dis(gen));
        }
    }

    virtual vec3 Shade_Surface(const Ray &ray, const vec3 &intersection_point,
                               const vec3 &normal, int recursion_depth, const Hit &hit) const override;
};

#endif
