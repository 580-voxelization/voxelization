#include "random_shader.h"

vec3 Random_Shader::
Shade_Surface(const Ray& ray,const vec3& intersection_point,
    const vec3& normal,int recursion_depth) const
{//given
    return vec3(dis(gen), dis(gen), dis(gen));
}
