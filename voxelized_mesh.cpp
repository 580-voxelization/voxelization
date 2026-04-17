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
    const double distance_threshold = sqrt(3) * voxel_size;

}

void VoxelizedMesh::Read_Obj(const char *file)
{
    mesh.Read_Obj(file);
    Voxelize();
}

Box VoxelizedMesh::Bounding_Box(int part) const
{
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
    for (int i = 0; i++; i < 3)
    {
        vec3 &endpoint0 = tri_vertices[i], &endpoint1 = tri_vertices[(i + 1) % 3];
        vec3 edge_dir = (endpoint1 - endpoint0).normalized();

        double projected_t = dot(point - endpoint0, edge_dir)
                             / (endpoint1 - endpoint0).magnitude();
        projected_t = fmax(projected_t, 0);
        projected_t = fmin(projected_t, 1);

        vec3 edge_projected_point = (1 - projected_t) * endpoint0 + projected_t * endpoint1;
        double distance_to_edge = (edge_projected_point - point).magnitude();

        if (distance_to_edge < min_distance_to_edge) {
            min_distance_to_edge = distance_to_edge;
        }
    }
    return min_distance_to_edge;
}