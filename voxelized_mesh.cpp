#include "voxelized_mesh.h"
#include <algorithm>
#include <cmath>
#include <limits>

// ---- Helper: Ray vs axis-aligned box. Returns true and t_near on hit. ----
static bool Ray_Box_Intersect(const Ray& ray, const vec3& lo, const vec3& hi, double& t_near)
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
    if (t_max < 0) return false;
    t_near = (t_min >= 0) ? t_min : t_max;
    return true;
}

Hit VoxelizedMesh::Intersection(const Ray& ray, int part) const
{
    Hit hit = {nullptr, 0.0, part};

    if (part >= 0 && part < (int)voxels.size()) {
        vec3 lo = voxels[part];
        vec3 hi = lo + vec3(voxel_size, voxel_size, voxel_size);
        double t;
        if (Ray_Box_Intersect(ray, lo, hi, t) && t >= small_t)
            hit = {this, t, part};
        return hit;
    }

    double t_box;
    if (!Ray_Box_Intersect(ray, box.lo, box.hi, t_box)) return hit;

    double min_t = std::numeric_limits<double>::max();
    for (size_t i = 0; i < voxels.size(); i++) {
        vec3 lo = voxels[i];
        vec3 hi = lo + vec3(voxel_size, voxel_size, voxel_size);
        double t;
        if (Ray_Box_Intersect(ray, lo, hi, t) && t >= small_t && t < min_t) {
            min_t = t;
            hit = {this, t, (int)i};
        }
    }
    return hit;
}

vec3 VoxelizedMesh::Normal(const vec3& point, int part) const
{
    vec3 lo = voxels[part];
    vec3 hi = lo + vec3(voxel_size, voxel_size, voxel_size);
    vec3 center = (lo + hi) * 0.5;
    vec3 d = point - center;

    int axis = 0;
    double best = std::abs(d[0]);
    for (int i = 1; i < 3; i++) {
        if (std::abs(d[i]) > best) { best = std::abs(d[i]); axis = i; }
    }
    vec3 n(0, 0, 0);
    n[axis] = (d[axis] > 0) ? 1.0 : -1.0;
    return n;
}

void VoxelizedMesh::Voxelize()
{
}

void VoxelizedMesh::Read_Obj(const char* file)
{
    mesh.Read_Obj(file);
    Voxelize();
}

Box VoxelizedMesh::Bounding_Box(int part) const
{
}
