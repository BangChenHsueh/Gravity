#pragma once
#include <vector>
#include <glad/glad.h>
#include <glm/glm.hpp>

class Sphere{
public:
    Sphere(float radius, int stacks, int slices);
    ~Sphere();

    void update(float deltaTime);
    void draw() const;

    glm::vec3 position;
    glm::vec3 color;
    glm::vec3 velocity;
    float speedMultiplier;

private:
    unsigned int VAO, VBO, EBO;
    unsigned int indexCount;

    void build(float radius, int stacks, int slices);
};