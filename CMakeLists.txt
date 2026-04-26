cmake_minimum_required(VERSION 3.10)
project(ray_tracer)
add_executable(ray_tracer main.cpp camera.cpp hierarchy.cpp flat_shader.cpp parse.cpp phong_shader.cpp plane.cpp reflective_shader.cpp render_world.cpp sphere.cpp box.cpp mesh.cpp voxelized_mesh.cpp random_shader.cpp)
target_compile_features(ray_tracer PRIVATE cxx_std_17)
