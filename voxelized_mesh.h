#ifndef __Voxelized_MESH_H__
#define __Voxelized_MESH_H__

#include "mesh.h"
#include <atomic>

struct BVH_Node
{
    Box bbox;
    int left;    
    int right;    
    int voxel_start; 
    int voxel_count;

    bool Is_Leaf() const { return left == -1; }
};

class VoxelizedMesh : public Object
{
    double voxel_size;
    std::vector<vec3> voxels;
    std::vector<std::vector<int>> voxels_to_triangles;
    Mesh mesh;

    Box box;
    bool use_sat = true;  // true = SAT，false = distance

    std::vector<BVH_Node> bvh_nodes;
    bool bvh_enabled;

    int Build_BVH(int start, int count);

    // Traverse BVH to find closest ray-voxel intersection
    void BVH_Intersect(const Ray& ray, int node_idx, double& min_t, Hit& hit) const;

    bool Triangle_Box_Intersect(int tri_idx, const vec3& lo, const vec3& hi) const;

    mutable std::atomic<long long> intersection_tests;

public:
    Box Triangle_Bounding_Box(int triangle_index) const;

    double Distance_To_Triangle(vec3 &point, int part) const;

public:
    VoxelizedMesh() : voxel_size(0.0f), bvh_enabled(true), intersection_tests(0) {}

    VoxelizedMesh(double voxel_size) : voxel_size(voxel_size), bvh_enabled(true), intersection_tests(0)
    {
    }

    virtual Hit Intersection(const Ray &ray, int part) const override;

    virtual vec3 Normal(const vec3 &point, int part) const override;

    void Voxelize();

    void Read_Obj(const char *file);

    Box Bounding_Box(int part) const override;

    void Set_Voxel_Size(float vs) { voxel_size = vs; }

    void Set_BVH_Enabled(bool enabled) { bvh_enabled = enabled; }

    int Voxel_Count() const { return (int)voxels.size(); }

    int BVH_Node_Count() const { return (int)bvh_nodes.size(); }

    void Reset_Stats() const { intersection_tests = 0; }

    long long Get_Intersection_Tests() const { return intersection_tests.load(); }
};

#endif
