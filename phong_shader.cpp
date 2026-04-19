#include <algorithm>
#include "light.h"
#include "phong_shader.h"
#include "ray.h"
#include "render_world.h"
#include "object.h"

vec3 Phong_Shader::
Shade_Surface(const Ray &ray, const vec3 &intersection_point,
              const vec3 &normal, int recursion_depth, const Hit &hit) const
{
    vec3 color;

//    TODO; //calculate the phong ambient + diffuse + specular
    color += color_ambient * world.ambient_intensity * world.ambient_color;
    for (Light *light: world.lights)
    {
        vec3 vector_to_light = light->position - intersection_point;
        if (world.enable_shadows)
        {
            Ray shadow_ray = Ray(intersection_point + small_t * vector_to_light.normalized(),
                                 vector_to_light.normalized());
            Hit shadow_hit = world.Closest_Intersection(shadow_ray);
            if (shadow_hit.object && shadow_hit.dist < vector_to_light.magnitude())
            {
                continue;
            }
        }

        vec3 light_color = light->Emitted_Light(vector_to_light);
        vec3 diffuse = color_diffuse * light_color * fmax(0, dot(vector_to_light.normalized(), normal));
        color += diffuse;

        vec3 reflected_vector = (2 * dot(vector_to_light, normal) * normal - vector_to_light).normalized();

        vec3 specular =
                color_specular * light_color * pow(fmax(0, dot(reflected_vector, -ray.direction)), specular_power);
        color += specular;
    }

    return color;
}