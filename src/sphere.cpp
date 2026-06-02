#include "sphere.h"
#include <cmath>

const float PI = 3.14159265359f;

Sphere::Sphere(float radius, int stacks, int slices)
    : 
    speedMultiplier(1.0f)
{
    this->radius = radius;
    position = glm::vec3(0.0f);
    color = glm::vec3(1.0f);
    velocity = glm::vec3(0.0f);
    build(radius, stacks, slices);
}

Sphere::~Sphere(){
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
}

void Sphere::build(float radius, int stacks, int slices){
    std::vector<float> verts;
    std::vector<unsigned int> indices;

    for(int i = 0; i <= stacks; ++i){
        float phi    = PI/2 - i * PI / stacks;
        float y      = radius * sin(phi);
        float cosPhi = cos(phi);

        for(int j = 0; j <= slices; ++j){
            float theta = j * 2*PI / slices;
            float x = radius * cosPhi * cos(theta);
            float z = radius * cosPhi * sin(theta);

            // position
            verts.push_back(x);
            verts.push_back(y);
            verts.push_back(z);
            // normal
            verts.push_back(x / radius);
            verts.push_back(y / radius);
            verts.push_back(z / radius);
        }
    }

    for(int i = 0; i < stacks; ++i){
        for(int j = 0; j < slices; ++j){
            unsigned int row1 = i     * (slices+1) + j;
            unsigned int row2 = (i+1) * (slices+1) + j;
            indices.push_back(row1);   indices.push_back(row2);   indices.push_back(row1+1);
            indices.push_back(row1+1); indices.push_back(row2);   indices.push_back(row2+1);
        }
    }

    indexCount = indices.size();

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, verts.size()*sizeof(float), verts.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size()*sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6*sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6*sizeof(float), (void*)(3*sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
}

void Sphere::draw() const {
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void Sphere::update(float deltaTime){
    float gravityConstant = -100.0f;
    velocity.y += gravityConstant*deltaTime;
    position += velocity * speedMultiplier * deltaTime;

    //ground detection(delete future, when using planets)
    float floorY = -2.0f;
    if((position.y - radius) <= floorY && velocity.y < 0.0f){
        position.y = (floorY + radius);

        velocity.y = -velocity.y * 1.0f;

        if(std::abs(velocity.y)<0.2f){
            velocity.y = 0.0f;
        }
    }
}

void Sphere::reset(){
    position = startPosition;
    velocity = startVelocity;
}