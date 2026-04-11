#include "plane.h"
#include "ray.h"
#include <cfloat>
#include <limits>

// Intersect with the half space defined by the plane.  The plane's normal
// points outside.  If the ray starts on the "inside" side of the plane, be sure
// to record a hit with t=0 as the first entry in hits.
Hit Plane::Intersection(const Ray &ray, int part) const {

//    TODO; //calculate ray+plane intersection
    Hit hit = {nullptr, 0, part};
    double d_dot_n = dot(ray.direction, normal);
    if (d_dot_n < 0) {
        double t = -dot(normal, ray.endpoint - x1) / d_dot_n;
        if (t >= small_t) {
            hit = {this, t, part};
        }
    } else if (d_dot_n > 0) {
        hit = {this, 0, part};
    }
    return hit;
}

vec3 Plane::Normal(const vec3 &point, int part) const {
    //normal is part of the plane so this one is a gimme
    return normal;
}

// There is not a good answer for the bounding box of an infinite object.
// The safe thing to do is to return a box that contains everything.
Box Plane::Bounding_Box(int part) const {
    //also a gimme
    Box b;
    b.hi.fill(std::numeric_limits<double>::max());
    b.lo = -b.hi;
    return b;
}
