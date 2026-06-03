#ifndef GRID_H
#define GRID_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <vector>

// #include "sphere.h"

class Sphere;

class Grid{
    public:
        struct Vertex{
            glm::vec3 position;
            glm::vec3 normal;
        };

    Grid(float minBound, float maxBound, float step, float yHeight);
    ~Grid();
    
    void draw(unsigned int shaderProgram, const glm::vec3& color, const std::vector<Sphere>& spheres);

    private:
        unsigned int VAO, VBO, EBO;
        GLsizei indexCount;
        std::vector<Vertex> baseVertices;
        float baselineY;
};

#endif