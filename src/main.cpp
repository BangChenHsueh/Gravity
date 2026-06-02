#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <cmath>
#include "sphere.h"

// ── GLM (header-only math library) ─────────────────────────────────────────
// Drop glm/ folder into dependencies/ and add to include_directories in CMake
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// ── Constants ───────────────────────────────────────────────────────────────
const unsigned int SCR_WIDTH  = 1600;
const unsigned int SCR_HEIGHT = 1200;

// ── Camera state ────────────────────────────────────────────────────────────
glm::vec3 cameraPos   = glm::vec3(0.0f, 0.0f, 3.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp    = glm::vec3(0.0f, 1.0f,  0.0f);

float yaw   = -90.0f;   // look along -Z initially
float pitch =   0.0f;
float lastX = SCR_WIDTH  / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool  firstMouse = true;
float fov = 90.0f;

float deltaTime = 0.0f;
float lastFrame = 0.0f;

// ── Callbacks ───────────────────────────────────────────────────────────────
void framebuffer_size_callback(GLFWwindow*, int w, int h){
    glViewport(0, 0, w, h);
}

void mouse_callback(GLFWwindow*, double xpos, double ypos){
    if(firstMouse){ lastX = xpos; lastY = ypos; firstMouse = false; }

    float xoffset = (xpos - lastX) * 0.1f;   // sensitivity
    float yoffset = (lastY - ypos) * 0.1f;   // reversed: y goes bottom→top
    lastX = xpos; lastY = ypos;

    yaw   += xoffset;
    pitch += yoffset;
    pitch  = glm::clamp(pitch, -89.0f, 89.0f);

    glm::vec3 dir;
    dir.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    dir.y = sin(glm::radians(pitch));
    dir.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    cameraFront = glm::normalize(dir);
}

void scroll_callback(GLFWwindow*, double, double yoffset){
    fov = glm::clamp(fov - (float)yoffset, 1.0f, 90.0f);
}

void process_input(GLFWwindow* window){
    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    float speed = 2.5f * deltaTime;
    if(glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) cameraPos += speed * cameraFront;
    if(glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) cameraPos -= speed * cameraFront;
    if(glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * speed;
    if(glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * speed;
}

// ── Shader helper ───────────────────────────────────────────────────────────
std::string loadFile(const char* path){
    std::ifstream f(path);
    if(!f.is_open()){ std::cerr << "Cannot open: " << path << "\n"; return ""; }
    std::stringstream ss; ss << f.rdbuf(); return ss.str();
}

unsigned int compileShader(GLenum type, const char* src){
    unsigned int id = glCreateShader(type);
    glShaderSource(id, 1, &src, NULL);
    glCompileShader(id);
    int ok; char log[512];
    glGetShaderiv(id, GL_COMPILE_STATUS, &ok);
    if(!ok){ glGetShaderInfoLog(id, 512, NULL, log); std::cerr << log << "\n"; }
    return id;
}

unsigned int buildProgram(const char* vertPath, const char* fragPath){
    std::string vs = loadFile(vertPath), fs = loadFile(fragPath);
    unsigned int v  = compileShader(GL_VERTEX_SHADER,   vs.c_str());
    unsigned int f  = compileShader(GL_FRAGMENT_SHADER, fs.c_str());
    unsigned int p  = glCreateProgram();
    glAttachShader(p, v); glAttachShader(p, f);
    glLinkProgram(p);
    int ok; char log[512];
    glGetProgramiv(p, GL_LINK_STATUS, &ok);
    if(!ok){ glGetProgramInfoLog(p, 512, NULL, log); std::cerr << log << "\n"; }
    glDeleteShader(v); glDeleteShader(f);
    return p;
}

// ── Main ────────────────────────────────────────────────────────────────────
int main(){
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Gravity – Sphere", NULL, NULL);
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window,       mouse_callback);
    glfwSetScrollCallback(window,          scroll_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);  // capture mouse

    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
    glEnable(GL_DEPTH_TEST);

    unsigned int shader = buildProgram(
        "D:/Code/Gravity/src/shader.vert",
        "D:/Code/Gravity/src/shader.frag"
    );

    // Build sphere mesh
    Sphere sphere(1.0f, 32, 32);
    sphere.position = glm::vec3(0.0f, 0.0f, 0.0f);
    sphere.color    = glm::vec3(0.3f, 0.6f, 1.0f);
    
    // std::vector<float>        verts;
    // std::vector<unsigned int> indices;
    // // buildSphere(1.0f, 32, 32, verts, indices);

    // unsigned int VAO, VBO, EBO;
    // glGenVertexArrays(1, &VAO);
    // glGenBuffers(1, &VBO);
    // glGenBuffers(1, &EBO);

    // glBindVertexArray(VAO);

    // glBindBuffer(GL_ARRAY_BUFFER, VBO);
    // glBufferData(GL_ARRAY_BUFFER, verts.size()*sizeof(float), verts.data(), GL_STATIC_DRAW);

    // glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    // glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size()*sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    // // layout 0 = position (xyz)
    // glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6*sizeof(float), (void*)0);
    // glEnableVertexAttribArray(0);
    // // layout 1 = normal (xyz)
    // glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6*sizeof(float), (void*)(3*sizeof(float)));
    // glEnableVertexAttribArray(1);

    // glBindVertexArray(0);

    // Render loop
    while(!glfwWindowShouldClose(window)){
        float current = (float)glfwGetTime();
        deltaTime = current - lastFrame;
        lastFrame = current;

        process_input(window);

        glClearColor(0.1f, 0.1f, 0.15f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(shader);

        // build model matrix from sphere's position
        glm::mat4 model = glm::translate(glm::mat4(1.0f), sphere.position);
        glUniformMatrix4fv(glGetUniformLocation(shader, "model"), 1, GL_FALSE, glm::value_ptr(model));
        glUniform3fv(glGetUniformLocation(shader, "objectColor"), 1, glm::value_ptr(sphere.color));



        // Matrices
        // glm::mat4 model      = glm::mat4(1.0f);
        glm::mat4 view       = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
        glm::mat4 projection = glm::perspective(glm::radians(fov),
                                 (float)SCR_WIDTH/SCR_HEIGHT, 0.1f, 100.0f);

                                 
        // glUniformMatrix4fv(glGetUniformLocation(shader, "model"),      1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(glGetUniformLocation(shader, "view"),       1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(shader, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

        // Lighting
        glUniform3f(glGetUniformLocation(shader, "lightPos"),    2.0f, 2.0f, 2.0f);
        glUniform3f(glGetUniformLocation(shader, "lightColor"),  1.0f, 1.0f, 1.0f);
        glUniform3f(glGetUniformLocation(shader, "objectColor"), 0.3f, 0.6f, 1.0f);

        sphere.draw();

        // glBindVertexArray(VAO);
        // glDrawElements(GL_TRIANGLES, (GLsizei)indices.size(), GL_UNSIGNED_INT, 0);
        // glBindVertexArray(0);P

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // glDeleteVertexArrays(1, &VAO);
    // glDeleteBuffers(1, &VBO);
    // glDeleteBuffers(1, &EBO);
    glDeleteProgram(shader);
    glfwTerminate();
    return 0;
}