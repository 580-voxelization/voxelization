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

// Ray-box test without distance for BVH node culling
static bool Ray_Box_Test(const Ray& ray, const Box& box)
{
    double t;
    return Ray_Box_Intersect(ray, box.lo, box.hi, t);
}

static const double bary_weight_tolerance = 1e-4;

// Build BVH recursively
// Returns index of the root node created
int Voxelized_Mesh::Build_BVH(int start, int count)
{
    BVH_Node node;
    node.left = node.right = -1;
    node.voxel_start = start;
    node.voxel_count = count;

    // Compute bounding box
    node.bbox.Make_Empty();
    for (int i = start; i < start + count; i++) {
        int voxel_index = bvh_voxel_indices[i];
        node.bbox.Include_Point(voxels[voxel_index]);
        node.bbox.Include_Point(voxels[voxel_index] + vec3(voxel_size, voxel_size, voxel_size));
    }

    // Leaf threshold: 4 voxels or fewer
    if (count <= 4) {
        int idx = (int)bvh_nodes.size();
        bvh_nodes.push_back(node);
        return idx;
    }

    // Find the longest axis of the bounding box
    vec3 extent = node.bbox.hi - node.bbox.lo;
    int axis = 0;
    if (extent[1] > extent[0]) axis = 1;
    if (extent[2] > extent[axis]) axis = 2;

    // Sort BVH indices along this axis, preserving the original voxel order.
    double vs = voxel_size;
    std::sort(bvh_voxel_indices.begin() + start, bvh_voxel_indices.begin() + start + count,
        [this, axis, vs](int a_index, int b_index) {
            const vec3& a = voxels[a_index];
            const vec3& b = voxels[b_index];
            return (a[axis] + vs * 0.5) < (b[axis] + vs * 0.5);
        });

    int mid = count / 2;

    int my_idx = (int)bvh_nodes.size();
    bvh_nodes.push_back(node); // placeholder

    int left_idx  = Build_BVH(start, mid);
    int right_idx = Build_BVH(start + mid, count - mid);

    bvh_nodes[my_idx].left  = left_idx;
    bvh_nodes[my_idx].right = right_idx;
    bvh_nodes[my_idx].voxel_start = -1;
    bvh_nodes[my_idx].voxel_count = 0;

    return my_idx;
}

void Voxelized_Mesh::BVH_Intersect(const Ray& ray, int node_idx, double& min_t, Hit& hit) const
{
    const BVH_Node& node = bvh_nodes[node_idx];

    double t_node;
    if (!Ray_Box_Intersect(ray, node.bbox.lo, node.bbox.hi, t_node))
        return;

    if (t_node >= min_t)
        return;

    if (node.Is_Leaf()) {
        // Test each voxel in this leaf
        for (int i = node.voxel_start; i < node.voxel_start + node.voxel_count; i++) {
            int voxel_index = bvh_voxel_indices[i];
            intersection_tests++;
            vec3 lo = voxels[voxel_index];
            vec3 hi = lo + vec3(voxel_size, voxel_size, voxel_size);
            double t;
            if (Ray_Box_Intersect(ray, lo, hi, t) && t >= small_t && t < min_t) {
                min_t = t;
                hit = {this, t, voxel_index};
            }
        }
    } else {
        double t_left = std::numeric_limits<double>::max();
        double t_right = std::numeric_limits<double>::max();
        Ray_Box_Intersect(ray, bvh_nodes[node.left].bbox.lo, bvh_nodes[node.left].bbox.hi, t_left);
        Ray_Box_Intersect(ray, bvh_nodes[node.right].bbox.lo, bvh_nodes[node.right].bbox.hi, t_right);

        if (t_left < t_right) {
            BVH_Intersect(ray, node.left,  min_t, hit);
            BVH_Intersect(ray, node.right, min_t, hit);
        } else {
            BVH_Intersect(ray, node.right, min_t, hit);
            BVH_Intersect(ray, node.left,  min_t, hit);
        }
    }
}

Hit Voxelized_Mesh::Intersection(const Ray &ray, int part) const
{
    Hit hit = {nullptr, 0.0, part};

    // Single-part intersection (used by hierarchy)
    if (part >= 0 && part < (int)voxels.size()) {
        intersection_tests++;
        vec3 lo = voxels[part];
        vec3 hi = lo + vec3(voxel_size, voxel_size, voxel_size);
        double t;
        if (Ray_Box_Intersect(ray, lo, hi, t) && t >= small_t)
            hit = {this, t, part};
        return hit;
    }

    double t_box;
    if (!Ray_Box_Intersect(ray, box.lo, box.hi, t_box)) return hit;

    if (bvh_enabled && !bvh_nodes.empty()) {
        // BVH
        double min_t = std::numeric_limits<double>::max();
        BVH_Intersect(ray, 0, min_t, hit);
    } else {
        // Brute-force
        double min_t = std::numeric_limits<double>::max();
        for (size_t i = 0; i < voxels.size(); i++) {
            intersection_tests++;
            vec3 lo = voxels[i];
            vec3 hi = lo + vec3(voxel_size, voxel_size, voxel_size);
            double t;
            if (Ray_Box_Intersect(ray, lo, hi, t) && t >= small_t && t < min_t) {
                min_t = t;
                hit = {this, t, (int)i};
            }
        }
    }
    return hit;
}

vec3 Voxelized_Mesh::Normal(const vec3 &point, int part) const
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

static bool Axis_Test(const vec3& axis,
                      const vec3& v0, const vec3& v1, const vec3& v2,
                      const vec3& h)
{
    double p0 = dot(v0, axis);
    double p1 = dot(v1, axis);
    double p2 = dot(v2, axis);

    double min_p = std::min(p0, std::min(p1, p2));
    double max_p = std::max(p0, std::max(p1, p2));

    double r = h[0] * std::abs(axis[0]) +
               h[1] * std::abs(axis[1]) +
               h[2] * std::abs(axis[2]);

    if (min_p > r || max_p < -r)
        return false;

    return true;
}

bool Voxelized_Mesh::Triangle_Box_Intersect(int tri_idx,
                                            const vec3& lo,
                                            const vec3& hi) const
{
    ivec3 tri = mesh.triangles[tri_idx];
    vec3 v0 = mesh.vertices[tri[0]];
    vec3 v1 = mesh.vertices[tri[1]];
    vec3 v2 = mesh.vertices[tri[2]];

    vec3 c = (lo + hi) * 0.5;
    vec3 h = (hi - lo) * 0.5;

    v0 -= c;
    v1 -= c;
    v2 -= c;

    vec3 e0 = v1 - v0;
    vec3 e1 = v2 - v1;
    vec3 e2 = v0 - v2;

    vec3 axes[9] = {
        cross(e0, vec3(1,0,0)), cross(e0, vec3(0,1,0)), cross(e0, vec3(0,0,1)),
        cross(e1, vec3(1,0,0)), cross(e1, vec3(0,1,0)), cross(e1, vec3(0,0,1)),
        cross(e2, vec3(1,0,0)), cross(e2, vec3(0,1,0)), cross(e2, vec3(0,0,1))
    };

    for (int i = 0; i < 9; i++) {
        if (axes[i].magnitude() < 1e-12) continue;
        if (!Axis_Test(axes[i], v0, v1, v2, h))
            return false;
    }

    for (int i = 0; i < 3; i++) {
        double min_v = std::min(v0[i], std::min(v1[i], v2[i]));
        double max_v = std::max(v0[i], std::max(v1[i], v2[i]));

        if (min_v > h[i] || max_v < -h[i])
            return false;
    }

    vec3 normal = cross(e0, e1);
    if (!Axis_Test(normal, v0, v1, v2, h))
        return false;

    return true;
}

void Voxelized_Mesh::Voxelize()
{
    voxels.clear();
    bvh_nodes.clear();
    voxels_to_triangles.clear();
    bvh_voxel_indices.clear();

    if (mesh.vertices.empty() || mesh.triangles.empty())
    {
        return;
    }

    box.Make_Empty();
    for (const vec3 &v: mesh.vertices)
    {
        box.Include_Point(v);
    }

    const double eps = 1e-9;
    box.lo -= vec3(eps, eps, eps);
    box.hi += vec3(eps, eps, eps);

    int nx = (int) std::ceil((box.hi[0] - box.lo[0]) / voxel_size);
    int ny = (int) std::ceil((box.hi[1] - box.lo[1]) / voxel_size);
    int nz = (int) std::ceil((box.hi[2] - box.lo[2]) / voxel_size);

    auto clampi = [](int v, int lo, int hi) -> int
    {
        return std::max(lo, std::min(v, hi));
    };

    const double distance_threshold = 0.5 * voxel_size;

    auto index = [=](int x, int y, int z) -> int
    {
        return x + nx * (y + ny * z);
    };

    // std::vector<unsigned char> occupied((size_t) nx * ny * nz, 0);
    std::vector<std::vector<int>> voxel_triangles((size_t) nx * ny * nz);

    for (int part = 0; part < (int)mesh.triangles.size(); ++part)
    {
        Box tri_box = Triangle_Bounding_Box(part);

        int x0 = (int) std::floor((tri_box.lo[0] - box.lo[0]) / voxel_size);
        int y0 = (int) std::floor((tri_box.lo[1] - box.lo[1]) / voxel_size);
        int z0 = (int) std::floor((tri_box.lo[2] - box.lo[2]) / voxel_size);

        int x1 = (int) std::floor((tri_box.hi[0] - box.lo[0]) / voxel_size);
        int y1 = (int) std::floor((tri_box.hi[1] - box.lo[1]) / voxel_size);
        int z1 = (int) std::floor((tri_box.hi[2] - box.lo[2]) / voxel_size);

        x0 = clampi(x0, 0, nx - 1);
        y0 = clampi(y0, 0, ny - 1);
        z0 = clampi(z0, 0, nz - 1);

        x1 = clampi(x1, 0, nx - 1);
        y1 = clampi(y1, 0, ny - 1);
        z1 = clampi(z1, 0, nz - 1);

        for (int z = z0; z <= z1; ++z)
            for (int y = y0; y <= y1; ++y)
                for (int x = x0; x <= x1; ++x)
                {
                    vec3 voxel_lo = box.lo + vec3(x * voxel_size, y * voxel_size, z * voxel_size);
                    vec3 center = voxel_lo + vec3(0.5 * voxel_size, 0.5 * voxel_size, 0.5 * voxel_size);

                    vec3 voxel_hi = voxel_lo + vec3(voxel_size, voxel_size, voxel_size);
                    size_t idx = (size_t) index(x, y, z);

                    bool hit = false;

                    if (sat_enabled) {
                        hit = Triangle_Box_Intersect(part, voxel_lo, voxel_hi);
                    } else {
                        vec3 p = center, target_point;
                        double d = Distance_To_Triangle(p, part, target_point);
                        hit = (d <= distance_threshold);
                    }

                    if (hit) {
                        voxel_triangles[idx].push_back(part);
                    }
                }
    }

    voxels.reserve((size_t) nx * ny * nz);
    voxels_to_triangles.reserve((size_t) nx * ny * nz);

    for (int z = 0; z < nz; ++z)
        for (int y = 0; y < ny; ++y)
            for (int x = 0; x < nx; ++x)
            {
                size_t idx = (size_t) index(x, y, z);

                if (!voxel_triangles[idx].empty())
                {
                    vec3 voxel_lo = box.lo + vec3(x * voxel_size, y * voxel_size, z * voxel_size);

                    voxels.push_back(voxel_lo);
                    voxels_to_triangles.push_back(voxel_triangles[idx]);
                }
            }

    number_parts = (int)voxels.size();
    box.Make_Empty();
    for (const vec3& v : voxels) {
        box.Include_Point(v);
        box.Include_Point(v + vec3(voxel_size, voxel_size, voxel_size));
    }

    if (!voxels.empty() && bvh_enabled) {
        bvh_voxel_indices.reserve(voxels.size());
        for (int i = 0; i < (int)voxels.size(); i++) {
            bvh_voxel_indices.push_back(i);
        }
        bvh_nodes.reserve(2 * voxels.size());
        Build_BVH(0, (int)voxels.size());
    }
}

void Voxelized_Mesh::Read_Obj(const char *file)
{
    mesh.Read_Obj(file);
    Voxelize();
}

Box Voxelized_Mesh::Bounding_Box(int part) const
{
    if (part >= 0 && part < (int)voxels.size())
    {
        Box voxel_box;
        voxel_box.lo = voxels[part];
        voxel_box.hi = voxels[part] + vec3(voxel_size, voxel_size, voxel_size);
        return voxel_box;
    }

    return box;
}

Box Voxelized_Mesh::Triangle_Bounding_Box(int triangle_index) const
{
    ivec3 triangle = mesh.triangles[triangle_index];

    const vec3 &a = mesh.vertices[triangle[0]];
    const vec3 &b = mesh.vertices[triangle[1]];
    const vec3 &c = mesh.vertices[triangle[2]];

    Box tri_box;
    tri_box.Make_Empty();
    tri_box.Include_Point(a);
    tri_box.Include_Point(b);
    tri_box.Include_Point(c);
    return tri_box;
}


double Voxelized_Mesh::Distance_To_Triangle(vec3 &point, int mesh_part, vec3 &target_point) const
{
    vec3 normal = mesh.Normal(vec3(), mesh_part);

    ivec3 triangle = mesh.triangles[mesh_part];
    vec3 tri_vertices[3] = {mesh.vertices[triangle[0]],
                            mesh.vertices[triangle[1]],
                            mesh.vertices[triangle[2]]};

    vec3 plane_projected_point = point + dot((tri_vertices[0] - point), normal) * normal;

    vec3 areas = {
            dot(cross(tri_vertices[2] - tri_vertices[1], plane_projected_point - tri_vertices[2]), normal),
            dot(cross(tri_vertices[0] - tri_vertices[2], plane_projected_point - tri_vertices[0]), normal),
            dot(cross(tri_vertices[1] - tri_vertices[0], plane_projected_point - tri_vertices[1]), normal)
    };
    double divider = areas[0] + areas[1] + areas[2];
    vec3 weights = {areas[0] / divider, areas[1] / divider, areas[2] / divider};

    // projected point is inside the triangle
    if (weights[0] > -bary_weight_tolerance && weights[1] > -bary_weight_tolerance &&
        weights[2] > -bary_weight_tolerance)
    {
        // distance to a point within the triangle
        target_point = plane_projected_point;
        return (plane_projected_point - point).magnitude();
    }

    // projected point is outside the triangle
    double min_distance_to_edge = (tri_vertices[0] - point).magnitude();
    for (int i = 0; i < 3; i++)
    {
        vec3 &endpoint0 = tri_vertices[i], &endpoint1 = tri_vertices[(i + 1) % 3];
        vec3 edge_dir = (endpoint1 - endpoint0).normalized();

        double projected_t = dot(point - endpoint0, edge_dir)
                             / (endpoint1 - endpoint0).magnitude();
        projected_t = fmax(projected_t, 0);
        projected_t = fmin(projected_t, 1);

        vec3 edge_projected_point = (1 - projected_t) * endpoint0 + projected_t * endpoint1;
        double distance_to_edge = (edge_projected_point - point).magnitude();

        if (distance_to_edge < min_distance_to_edge)
        {
            target_point = edge_projected_point;
            min_distance_to_edge = distance_to_edge;
        }
    }
    return min_distance_to_edge;
}
