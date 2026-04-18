#include "voxelized_mesh.h"
#include <cmath>

// Consider a triangle to intersect a ray if the ray intersects the plane of the
// triangle with barycentric weights in [-weight_tolerance, 1+weight_tolerance]
static const double bary_weight_tolerance = 1e-4;

static const double lerp_weight_tolerance = 1e-4;

Hit VoxelizedMesh::Intersection(const Ray &ray, int part) const
{
}

vec3 VoxelizedMesh::Normal(const vec3 &point, int part) const
{
}

void VoxelizedMesh::Voxelize()
{
    voxels.clear();

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

    std::vector<unsigned char> occupied((size_t) nx * ny * nz, 0);

    for (int part = 0; part < mesh.triangles.size(); ++part)
    {
        Box tri_box = Bounding_Box(part);

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

                    vec3 p = center;
                    double d = Distance_To_Triangle(p, part);

                    if (d <= distance_threshold)
                    {
                        occupied[(size_t) index(x, y, z)] = 1;
                    }
                }
    }

    voxels.reserve((size_t) nx * ny * nz);
    for (int z = 0; z < nz; ++z)
        for (int y = 0; y < ny; ++y)
            for (int x = 0; x < nx; ++x)
            {
                if (occupied[(size_t) index(x, y, z)])
                {
                    voxels.push_back(box.lo + vec3(x * voxel_size, y * voxel_size, z * voxel_size));
                }
            }


}

void VoxelizedMesh::Read_Obj(const char *file)
{
    mesh.Read_Obj(file);
    Voxelize();
}

Box VoxelizedMesh::Bounding_Box(int part) const
{
    ivec3 triangle = mesh.triangles[part];

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


double VoxelizedMesh::Distance_To_Triangle(vec3 &point, int part) const
{
    vec3 normal = mesh.Normal(vec3(), part);

    ivec3 triangle = mesh.triangles[part];
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
            min_distance_to_edge = distance_to_edge;
        }
    }
    return min_distance_to_edge;
}