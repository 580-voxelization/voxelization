#include "mesh.h"
#include <fstream>
#include <string>
#include <limits>

// Consider a triangle to intersect a ray if the ray intersects the plane of the
// triangle with barycentric weights in [-weight_tolerance, 1+weight_tolerance]
static const double weight_tolerance = 1e-4;

// Read in a mesh from an obj file.  Populates the bounding box and registers
// one part per triangle (by setting number_parts). Given.
void Mesh::Read_Obj(const char *file) {
    std::string obj_path = "./input/obj/" + std::string(file);
    std::ifstream fin(obj_path.c_str());
    if (!fin) {
        exit(EXIT_FAILURE);
    }
    std::string line;
    ivec3 e;
    vec3 v;
    box.Make_Empty();
    while (fin) {
        getline(fin, line);

        if (sscanf(line.c_str(), "v %lg %lg %lg", &v[0], &v[1], &v[2]) == 3) {
            vertices.push_back(v);
            box.Include_Point(v);
        }

        if (sscanf(line.c_str(), "f %d %d %d", &e[0], &e[1], &e[2]) == 3) {
            for (int i = 0; i < 3; i++) e[i]--;
            triangles.push_back(e);
        }
    }
    number_parts = triangles.size();
}

// Check for an intersection against the ray.  See the base class for details.
Hit Mesh::Intersection(const Ray &ray, int part) const {

//    TODO; //implement Mesh+ray Intersection
    Hit hit = {nullptr, 0.0, part};
    if (part < 0) {
        double min_t = std::numeric_limits<double>::max();
        for (int tri = 0; tri < triangles.size(); tri++) {
            double dist;
            if (Intersect_Triangle(ray, tri, dist) && dist >= small_t && dist < min_t) {
                min_t = dist;
                hit = {this, min_t, tri};
            }
        }
    } else {
        double dist;
        if (Intersect_Triangle(ray, part, dist) && dist >= small_t) {
            hit = {this, dist, part};
        }
    }

    return hit;
}

// Compute the normal direction for the triangle with index part.
vec3 Mesh::Normal(const vec3 &point, int part) const {
    assert(part >= 0);

    ivec3 current_triangle = triangles[part];

    vec3 tri_normal;

//    TODO;
    //implement tri normal calculation

    vec3 current_vertices[3] = {vertices[current_triangle[0]],
                                vertices[current_triangle[1]],
                                vertices[current_triangle[2]]};
    tri_normal = cross(current_vertices[1] - current_vertices[0],
                       current_vertices[2] - current_vertices[1]).normalized();

    return tri_normal;
}

// This is a helper routine whose purpose is to simplify the implementation
// of the Intersection routine.  It should test for an intersection between
// the ray and the triangle with index tri.  If an intersection exists,
// record the distance and return true.  Otherwise, return false.
// This intersection should be computed by determining the intersection of
// the ray and the plane of the triangle.  From this, determine (1) where
// along the ray the intersection point occurs (dist) and (2) the barycentric
// coordinates within the triangle where the intersection occurs.  The
// triangle intersects the ray if dist>small_t and the barycentric weights are
// larger than -weight_tolerance.  The use of small_t avoid the self-shadowing
// bug, and the use of weight_tolerance prevents rays from passing in between
// two triangles.
bool Mesh::Intersect_Triangle(const Ray &ray, int tri, double &dist) const {
    ivec3 points = triangles[tri];

//    TODO;
    //implement tri+ray intersection
    vec3 normal = Normal({0, 0, 0}, tri);
    vec3 current_vertices[3] = {vertices[points[0]],
                                vertices[points[1]],
                                vertices[points[2]]};
    double d_dot_n = dot(ray.direction, normal);
    double t = 0;
    if (d_dot_n != 0) {
        t = -dot(normal, ray.endpoint - current_vertices[0]) / d_dot_n;
    }

    if (t >= small_t) {
        vec3 intersection_point = ray.Point(t);
        vec3 areas = {
                dot(cross(current_vertices[2] - current_vertices[1], intersection_point - current_vertices[2]), normal),
                dot(cross(current_vertices[0] - current_vertices[2], intersection_point - current_vertices[0]), normal),
                dot(cross(current_vertices[1] - current_vertices[0], intersection_point - current_vertices[1]), normal)
        };
        double divider = areas[0] + areas[1] + areas[2];
        vec3 weights = {areas[0] / divider, areas[1] / divider, areas[2] / divider};
        if (weights[0] > -weight_tolerance && weights[1] > -weight_tolerance && weights[2] > -weight_tolerance) {
            dist = t;
            return true;
        }
    }

    return false;
}

// Compute the bounding box.  Return the bounding box of only the triangle whose
// index is part.
Box Mesh::Bounding_Box(int part) const {
    Box b;
    TODO;
    return b;
}
