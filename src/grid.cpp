#include "grid.h"
#include <glm/gtc/type_ptr.hpp>

Grid::Grid(float minBound, float maxBound, float step, float yHeight){
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

    //grid resolution boundaries
    int cols = static_cast<int>((maxBound - minBound)/step)+1;
    int rows = cols;

    for (int r = 0; r < rows; ++r) {
        float z = minBound + (r * step); // Maps clean steps from -30.0f up to 30.0f
        
        for (int c = 0; c < cols; ++c) {
            float x = minBound + (c * step);
            
            Vertex v;
            v.position = glm::vec3(x, yHeight, z);
            v.normal   = glm::vec3(0.0f, 1.0f, 0.0f);
            vertices.push_back(v);
        }
    }

    for(int r = 0; r < rows; r++){
        for(int c = 0; c < cols; c++){
            unsigned int current = r * cols + c;

            if(c < cols - 1){
                indices.push_back(current);
                indices.push_back(current + 1);
            }

            if(r < rows - 1){
                indices.push_back(current);
                indices.push_back(current + cols);
            }
        }
    }

    indexCount = static_cast<GLsizei>(indices.size());

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)sizeof(glm::vec3));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
}

Grid::~Grid(){
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
}

void Grid::draw(unsigned int shaderProgram, const glm::vec3& color){
    glm::mat4 model = glm::mat4(1.0f);
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
    glUniform3fv(glGetUniformLocation(shaderProgram, "objectColor"), 1, glm::value_ptr(color));

    glBindVertexArray(VAO);
    glDrawElements(GL_LINES, indexCount, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}