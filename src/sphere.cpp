#include "sphere.h"
#include <cmath>
#include <algorithm>

const float PI = 3.14159265359f;

static glm::vec4 getMetricDiagonal(const glm::vec4& pos4D, float starMass, const glm::vec4& starPos4D, float c, float G){
    //table
    //[0] -> t
    //[1] -> x
    //[2] -> y
    //[3] -> z
    
    float dx = pos4D[1] - starPos4D[1];
    float dy = pos4D[2] - starPos4D[2];
    float dz = pos4D[3] - starPos4D[3];
    float r = std::sqrt(dx*dx + dy*dy + dz*dz);

    if(r < 1.0f) r = 1.0f;

    float ShwarzschildFactor = 1-((2*G*starMass)/(c*c*r));

    return glm::vec4(
        -1*ShwarzschildFactor*c*c,
        1/ShwarzschildFactor,
        1/ShwarzschildFactor,
        1/ShwarzschildFactor
    );
}

void Sphere::updateTensorPhysics(float deltaTime, float starMass, const glm::vec4& starPos, float c, float G){
    if(this->isStaticStar){
        return;
    }

    float h = 0.01f;
    float dg[4][4][4] = { 0 };
    float Christoffel[4][4][4] = { 0 };

    //spatial derivatives
    for(int alpha = 1; alpha < 4; ++alpha){
        glm::vec4 posPlus = this->position4D; posPlus[alpha] += h;
        glm::vec4 posMinus = this->position4D; posMinus[alpha] -= h;

        glm::vec4 gPlus = getMetricDiagonal(posPlus, starMass, starPos, c, G);
        glm::vec4 gMinus = getMetricDiagonal(posMinus, starMass, starPos, c, G);

        for(int mu = 0; mu < 4; ++mu){
            dg[alpha][mu][mu] = (gPlus[mu] - gMinus[mu])/(2.0f * h);
        }
    }

    //get Inverse g
    glm::vec4 gCurrent = getMetricDiagonal(this->position4D, starMass, starPos, c, G);
    glm::vec4 gInverse = glm::vec4(1.0f/gCurrent[0], 1.0f/gCurrent[1], 1.0f/gCurrent[2], 1.0f/gCurrent[3]);

    //construct Christoffel
    for(int mu = 0; mu < 4; ++mu){
        for(int alpha = 0; alpha < 4; ++alpha){
            for(int beta = 0; beta < 4; ++beta){
                int sigma = mu;
                float term1 = dg[alpha][sigma][beta];
                float term2 = dg[beta][sigma][alpha];
                float term3 = dg[sigma][alpha][beta];

                Christoffel[mu][alpha][beta] = 0.5f * gInverse[mu] * (term1 + term2 - term3);
            }
        }
    }

    //solve geodesic equation for acceleration
    glm::vec4 acceleration4D = glm::vec4(0.0f);
    for(int mu = 0; mu < 4; ++mu){
        float contractionSum = 0.0f;
        for(int alpha = 0; alpha < 4; ++alpha){
            for(int beta = 0; beta < 4; ++beta){
                contractionSum += Christoffel[mu][alpha][beta] * this->velocity4D[alpha] * this->velocity4D[beta];
            }
        }
        acceleration4D[mu] = -contractionSum;
    }

    //integrate position
    this->velocity4D += acceleration4D * deltaTime;
    this->position4D += velocity4D * deltaTime;
}

Sphere::Sphere(float radius, int stacks, int slices, float mass)
    : 
    speedMultiplier(1.0f),
    mass(mass),
    accumulatedForce(0.0f)

{
    this->radius = radius;
    this->isStaticStar = false;
    position4D = glm::vec4(0.0f);
    color = glm::vec3(1.0f);
    velocity4D = glm::vec4(0.0f);
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
    // glm::vec3 acceleration = accumulatedForce/mass;

    // velocity += acceleration*deltaTime;
    // position += velocity * speedMultiplier * deltaTime;

    // //ground detection(delete future, when using planets)
    // // float floorY = -2.0f;
    // if(velocity.y < 0.0f){
    //     velocity.y = -velocity.y;

    //     if(std::abs(velocity.y)<0.2f){
    //         velocity.y = 0.0f;
    //     }
    // }
}

void Sphere::reset(){
    position4D = startPosition;
    velocity4D = startVelocity;
}

void Sphere::calculateGravity(std::vector<Sphere>& spheres, float G){
    // for(auto& s : spheres){
    //     s.accumulatedForce = glm::vec3(0.0f);
    // }

    // for(int i = 0; i < spheres.size(); i++){
    //     for(int j = i + 1; j < spheres.size(); j++){
    //         glm::vec3 direction = spheres[j].position - spheres[i].position;
    //         float distance = glm::length(direction);

    //         if(distance < 0.1f) distance = 0.1f;

    //         //f = gmm/r^2
    //         float forceMagnitude = (G * spheres[j].mass * spheres[i].mass)/(distance*distance);
    //         glm::vec3 forceVector = glm::normalize(direction) * forceMagnitude;

    //         spheres[i].accumulatedForce += forceVector;
    //         spheres[j].accumulatedForce -= forceVector;
    //     }
    // }
}