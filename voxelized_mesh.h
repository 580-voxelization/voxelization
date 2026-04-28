#ifndef __Voxelized_MESH_H__
#define __Voxelized_MESH_H__

#include "mesh.h"
#include <atomic>

class Voxel_Uniform_Shader;
class Voxel_Face_Uniform_Shader;
class Voxel_Frame_Shader;

struct BVH_Node
{
    Box bbox;
    int left;
    int right;
    int voxel_start;
    int voxel_count;

    bool Is_Leaf() const { return left == -1; }
};

class Voxelized_Mesh : public Object
{
    double voxel_size;
    std::vector<vec3> voxels;
    std::vector<std::vector<int>> voxels_to_triangles;
    Mesh mesh;

    Box box;


    std::vector<BVH_Node> bvh_nodes;
    std::vector<int> bvh_voxel_indices;
    bool bvh_enabled;

    int Build_BVH(int start, int count);

    // Traverse BVH to find closest ray-voxel intersection
    void BVH_Intersect(const Ray &ray, int node_idx, double &min_t, Hit &hit) const;

    bool Triangle_Box_Intersect(int tri_idx, const vec3 &lo, const vec3 &hi) const;

    mutable std::atomic<long long> intersection_tests;

    bool sat_enabled = true;

public:
    Box Triangle_Bounding_Box(int triangle_index) const;

    double Distance_To_Triangle(vec3 &point, int mesh_part, vec3 &target_point) const;


public:
    Voxelized_Mesh() : voxel_size(0.0f), bvh_enabled(false), intersection_tests(0) {}

    Voxelized_Mesh(double voxel_size, bool sat_enabled) : voxel_size(voxel_size), bvh_enabled(false),
                                                          sat_enabled(sat_enabled), intersection_tests(0)
    {
    }

    virtual Hit Intersection(const Ray &ray, int part) const override;

    virtual vec3 Normal(const vec3 &point, int part) const override;

    void Voxelize();

    void Read_Obj(const char *file);

    Box Bounding_Box(int part) const override;

    void Set_Voxel_Size(float vs) { voxel_size = vs; }

    void Set_BVH_Enabled(bool enabled) {
        bvh_enabled = enabled;
        if (enabled && bvh_nodes.empty() && !voxels.empty()) {
            bvh_voxel_indices.clear();
            bvh_voxel_indices.reserve(voxels.size());
            for (int i = 0; i < (int)voxels.size(); i++)
                bvh_voxel_indices.push_back(i);
            bvh_nodes.reserve(2 * voxels.size());
            Build_BVH(0, (int)voxels.size());
        }
    }

    int Voxel_Count() const { return (int) voxels.size(); }

    int BVH_Node_Count() const { return (int) bvh_nodes.size(); }

    void Reset_Stats() const { intersection_tests = 0; }

    long long Get_Intersection_Tests() const { return intersection_tests.load(); }

    friend Voxel_Uniform_Shader;
    friend Voxel_Face_Uniform_Shader;
    friend Voxel_Frame_Shader;
};

#endif