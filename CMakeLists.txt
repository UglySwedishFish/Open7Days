cmake_minimum_required(VERSION 3.8.2)
project(Open7Days)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(SFML COMPONENTS graphics REQUIRED)
find_package(glm REQUIRED)
find_package(assimp REQUIRED)

add_executable(7days
./WaterRenderer.cpp
./Framebuffer.cpp
./Core.cpp
./IndirectDiffuse.cpp
./Materials.cpp
./Window.cpp
./Texture.cpp
./Voxelizer.cpp
./Mesh.cpp
./Frustum.cpp
./Pipeline.cpp
./CubeMapRenderer.cpp
./Shadows.cpp
./main.cpp
./IndirectSpecular.cpp
./FirstPassLighting.cpp
./Chunk.cpp
./Camera.cpp
./LightCombiner.cpp
./Shader.cpp
./DeferredRenderer.cpp
./GL/glad.c)

target_link_libraries(7days PRIVATE sfml-graphics ${ASSIMP_LIBRARIES} ${CMAKE_DL_LIBS})
