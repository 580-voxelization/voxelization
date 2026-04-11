#include "reflective_shader.h"
#include "ray.h"
#include "render_world.h"

vec3 Reflective_Shader::
Shade_Surface(const Ray &ray, const vec3 &intersection_point,
              const vec3 &normal, int recursion_depth) const {
    vec3 color = shader->Shade_Surface(ray, intersection_point, normal, recursion_depth);
    color *= 1 - reflectivity;
    if (recursion_depth >= world.recursion_depth_limit) {
        return color;
    }

//    TODO;
    //recursively cast ray untill recursion_depth is reached;
    vec3 reflected_direction = (ray.direction - 2 * dot(ray.direction, normal) * normal).normalized();
    Ray reflected_ray = Ray(intersection_point + small_t * reflected_direction, reflected_direction);

    //combine results into color;


    vec3 reflected_color = world.Cast_Ray(reflected_ray, recursion_depth + 1);
    color += reflectivity * reflected_color;

    return color;
}
