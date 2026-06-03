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

    glm::vec4 position4D;
    glm::vec3 color;
    glm::vec4 velocity4D;
    glm::vec4 startPosition;
    glm::vec4 startVelocity;
    glm::vec4 accumulatedForce;
    float speedMultiplier;
    float radius;
    float mass;
    bool isStaticStar;

    void updateTensorPhysics(float deltaTime, float starMass, const glm::vec4& starPos, float c, float G);

private:
    unsigned int VAO, VBO, EBO;
    unsigned int indexCount;

    void build(float radius, int stacks, int slices);
};