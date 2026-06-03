#ifndef GRID_H
#define GRID_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <vector>

class Grid{
    public:
        struct Vertex{
            glm::vec3 position;
            glm::vec3 normal;
        };

    Grid(float minBound, float maxBound, float step, float yHeight);
    ~Grid();
    
    void draw(unsigned int shaderProgram, const glm::vec3& color);

    private:
        unsigned int VAO, VBO, EBO;
        GLsizei indexCount;
};

#endif