#include <limits>
#include "box.h"

// Return whether the ray intersects this box.
bool Box::Intersection(const Ray& ray) const
{
    double t_min = -std::numeric_limits<double>::infinity();
    double t_max =  std::numeric_limits<double>::infinity();
    for (int i = 0; i < 3; i++) {
        if (std::abs(ray.direction[i]) < 1e-12) {
            if (ray.endpoint[i] < lo[i] || ray.endpoint[i] > hi[i]) return false;
        } else {
            double inv_d = 1.0 / ray.direction[i];
            double t1 = (lo[i] - ray.endpoint[i]) * inv_d;
            double t2 = (hi[i] - ray.endpoint[i]) * inv_d;
            if (t1 > t2) std::swap(t1, t2);
            t_min = std::max(t_min, t1);
            t_max = std::min(t_max, t2);
            if (t_min > t_max) return false;
        }
    }
    return t_max >= 0;
}

// Compute the smallest box that contains both *this and bb.
Box Box::Union(const Box& bb) const
{
    Box box;
    box.lo = componentwise_min(lo, bb.lo);
    box.hi = componentwise_max(hi, bb.hi);
    return box;
}

// Enlarge this box (if necessary) so that pt also lies inside it.
void Box::Include_Point(const vec3& pt)
{
    lo = componentwise_min(lo, pt);
    hi = componentwise_max(hi, pt);
}

// Create a box to which points can be correctly added using Include_Point.
void Box::Make_Empty()
{
    lo.fill(std::numeric_limits<double>::infinity());
    hi=-lo;
}
