#include "voxelized_mesh.h"

Hit VoxelizedMesh::Intersection(const Ray& ray, int part) const
{
}

vec3 VoxelizedMesh::Normal(const vec3& point, int part) const
{
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
