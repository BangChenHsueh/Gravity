cmake_minimum_required(VERSION 3.10)
project(GravityWindow)
set(CMAKE_CXX_STANDARD 17)

#add dependencies(GLAD and GLFW)
#GLFW
add_subdirectory(dependencies/glfw-3.4)

#GLAD
add_library(glad dependencies/glad/src/glad.c)
target_include_directories(glad PUBLIC dependencies/glad/include)

include_directories(
    dependencies/glad/include
    dependencies/glfw-3.4/include
    dependencies/glm
)

add_executable(GravityWindow 
    src/main.cpp
    src/sphere.h
    src/sphere.cpp
)

target_link_libraries(GravityWindow glfw glad opengl32)

