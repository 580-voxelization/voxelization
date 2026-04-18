#ifndef __Voxelized_MESH_H__
#define __Voxelized_MESH_H__

#include "mesh.h"

class VoxelizedMesh : public Object
{
    float voxel_size;
    // lo vertices of voxelized cubes
    std::vector<vec3> voxels;
    Mesh mesh;

    Box box;
    
public:
    VoxelizedMesh() : voxel_size(0.0f) {}


    virtual Hit Intersection(const Ray& ray, int part) const override;
    virtual vec3 Normal(const vec3& point, int part) const override;
    void Voxelize();
    void Read_Obj(const char* file);
    Box Bounding_Box(int part) const override;

    void Set_Voxel_Size(float vs) { voxel_size = vs; }
};
#endif
