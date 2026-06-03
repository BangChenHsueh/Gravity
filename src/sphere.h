#pragma once
#include <vector>
#include <glad/glad.h>
#include <glm/glm.hpp>

class Sphere{
public:
    Sphere(float radius, int stacks, int slices, float mass);
    ~Sphere();

    static void calculateGravity(std::vector<Sphere>& spheres, float G);

    void update(float deltaTime);
    void draw() const;
    void reset();

    glm::vec3 position;
    glm::vec3 color;
    glm::vec3 velocity;
    glm::vec3 startPosition;
    glm::vec3 startVelocity;
    glm::vec3 accumulatedForce;
    float speedMultiplier;
    float radius;
    float mass;

private:
    unsigned int VAO, VBO, EBO;
    unsigned int indexCount;

    void build(float radius, int stacks, int slices);
};