#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <cmath>
#include "sphere.h"
#include "grid.h"

// ── GLM (header-only math library) ─────────────────────────────────────────
// Drop glm/ folder into dependencies/ and add to include_directories in CMake
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// ── Constants ───────────────────────────────────────────────────────────────
const unsigned int SCR_WIDTH  = 1600;
const unsigned int SCR_HEIGHT = 1200;
const float GRAVITY = -9.8f;
const float G = 0.0667f;
float c = 50.0f;          // Speed of light in engine scale
float starMass = 18740.0f;

// ── Camera state ────────────────────────────────────────────────────────────
glm::vec3 cameraPos   = glm::vec3(0.0f, 10.0f, -6.0f);
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

    float speed = 20.0f * deltaTime;
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
    
    //creates grid
    Grid flatPlane(-200.0f, 200.0f, 1.0f, 4.0f);

    //creating spheres list
    std::vector<Sphere> spheres;

    spheres.reserve(2);

    spheres.emplace_back(1.0f, 32, 32, 1.0f);     // Index 0: Orbiting Planet
    spheres.emplace_back(1.0f, 32, 32, starMass); // Index 1: Static Star

    spheres[0].color = glm::vec3(0.0f, 1.0f, 0.5f); // Cyan Planet
    spheres[1].color = glm::vec3(0.05f, 0.05f, 0.05f); // Red Star

    //static star
    spheres[1].isStaticStar = false;
    spheres[1].position4D = glm::vec4(0.0f, 50.0f, 4.0f, -6.0f);
    spheres[1].velocity4D = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);

    //orbiting planets
    spheres[0].isStaticStar = false;
    spheres[0].position4D = glm::vec4(0.0f, 35.0f, 4.0f, -6.0f);
    spheres[0].velocity4D = glm::vec4(0.0f, 0.0f, 0.0f, 9.3f);

    //velocity invariant calculation
    float dx = spheres[0].position4D[1] - spheres[1].position4D[1];
    float dy = spheres[0].position4D[2] - spheres[1].position4D[2];
    float dz = spheres[0].position4D[3] - spheres[1].position4D[3];
    float rStart = std::sqrt(dx*dx + dy*dy + dz*dz);

    //Schwarzschild factor
    float factor = 1.0f - ((2.0f*G*starMass)/(c*c*rStart));

    //calculate spatial velocity
    float spatialTerm = (1.0f/factor) * (spheres[0].velocity4D[1]*spheres[0].velocity4D[1]+
                                         spheres[0].velocity4D[2]*spheres[0].velocity4D[2]+
                                         spheres[0].velocity4D[3]*spheres[0].velocity4D[3]);

    spheres[0].velocity4D[0] = std::sqrt((c * c + spatialTerm) / (factor * c * c));

    // Save state for resets
    spheres[0].startPosition = spheres[0].position4D;
    spheres[0].startVelocity = spheres[0].velocity4D;
    spheres[1].startPosition = spheres[1].position4D;
    spheres[1].startVelocity = spheres[1].velocity4D;
    
    //reset clock
    lastFrame = (float)glfwGetTime();

    // Render loop
    while(!glfwWindowShouldClose(window)){
        float current = (float)glfwGetTime();
        deltaTime = current - lastFrame;
        lastFrame = current;

        process_input(window);

        glClearColor(0.1f, 0.1f, 0.15f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(shader);

        // sphere.update(deltaTime);

        // build model matrix from sphere's position
        // glm::mat4 model = glm::translate(glm::mat4(1.0f), sphere.position);
        // glUniformMatrix4fv(glGetUniformLocation(shader, "model"), 1, GL_FALSE, glm::value_ptr(model));
        // glUniform3fv(glGetUniformLocation(shader, "objectColor"), 1, glm::value_ptr(sphere.color));

        // Matrices
        // glm::mat4 model      = glm::mat4(1.0f);
        glm::mat4 view       = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
        glm::mat4 projection = glm::perspective(glm::radians(fov),
        (float)SCR_WIDTH/SCR_HEIGHT, 0.1f, 1000.0f);
              
        // glUniformMatrix4fv(glGetUniformLocation(shader, "model"),      1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(glGetUniformLocation(shader, "view"),       1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(shader, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

        // Lighting
        glUniform3f(glGetUniformLocation(shader, "lightPos"),    2.0f, 2.0f, 2.0f);
        glUniform3f(glGetUniformLocation(shader, "lightColor"),  1.0f, 1.0f, 1.0f);
        // glUniform3f(glGetUniformLocation(shader, "objectColor"), 0.3f, 0.6f, 1.0f);

        // sphere.draw();

        //draw grid
        glUniform1i(glGetUniformLocation(shader, "useLighting"), 0);
        flatPlane.draw(shader, glm::vec3(1.0f, 1.0f, 1.0f), spheres);

        //calculate force of gravity
        // Sphere::calculateGravity(spheres, G);

        glm::vec4 starsPos4D = spheres[1].position4D;

        glUniform1i(glGetUniformLocation(shader, "useLighting"), 1);
        for(int i = 0; i < spheres.size(); i++){
            // spheres[i].update(deltaTime);

            spheres[i].updateTensorPhysics(deltaTime, starMass, starsPos4D, c, G);

            // 1. Start with an identity matrix
            glm::mat4 model = glm::mat4(1.0f);

            model = glm::translate(model, glm::vec3(spheres[i].position4D[1],
                                                    spheres[i].position4D[2],
                                                    spheres[i].position4D[3]));
        
            model = glm::scale(model, glm::vec3(spheres[i].radius));

            // 4. Send matrix to shader pipeline
            glUniformMatrix4fv(glGetUniformLocation(shader, "model"), 1, GL_FALSE, glm::value_ptr(model));
            glUniform3fv(glGetUniformLocation(shader, "objectColor"), 1, glm::value_ptr(spheres[i].color));

            spheres[i].draw();

        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteProgram(shader);
    glfwTerminate();
    return 0;
}