#include "sphere.h"
#include "ray.h"

// Determine if the ray intersects with the sphere
Hit Sphere::Intersection(const Ray &ray, int part) const {
//    TODO; //calculate ray sphere intersection
    Hit hit = {nullptr, 0.0, part};

    vec3 l = ray.endpoint - center;
    double b = 2 * dot(ray.direction, l);
    double delta = pow(b, 2) - 4 * (pow(l.magnitude(), 2) - pow(radius, 2));
    if (delta > 0) {
        double sqrt_delta = sqrt(delta);
        double t1 = (-b - sqrt_delta) / 2, t2 = (-b + sqrt_delta) / 2;
        if (t1 >= small_t) {
            hit = {this, t1, part};
        } else if (t2 >= small_t) {
            hit = {this, t2, part};
        }
    }

    return hit;
}

vec3 Sphere::Normal(const vec3 &point, int part) const {
    vec3 normal;

//    TODO; //calculate Sphere surface normal at point
    normal = (point - center).normalized();

    return normal;
}

Box Sphere::Bounding_Box(int part) const {
    Box box;
    TODO; // calculate bounding box
    return box;
}
