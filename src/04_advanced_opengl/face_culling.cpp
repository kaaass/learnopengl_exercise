#include <iostream>
#include <map>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "../shader_s.h"
#include "../model.h"
#include "../camera.h"

#include "block_and_plane_vertices.h"

const char* vertShaderPath = "shader/depth_testing.vs";
const char* fragShaderPath = "shader/plain.fs";

int screenWidth = 1280;
int screenHeight = 720;

Shader* shader = nullptr;
Shader* outlineShader = nullptr;

unsigned int cubeVAO, cubeVBO;
unsigned int planeVAO, planeVBO;
unsigned int vegetationVAO, vegetationVBO;
unsigned int cubeTexture;
unsigned int floorTexture;
unsigned int grassTexture;
Model* model = nullptr;

float deltaTime = 0.0f;
float lastFrame = 0.0f;

Camera* camera = nullptr;
float lastX = screenWidth / 2;
float lastY = screenHeight / 2;
bool firstMouse = true;

vector<glm::vec3> vegetation;

void prepareDraw() {
    // Create shader
    shader = new Shader(vertShaderPath, fragShaderPath);
    // Create camera
    camera = new Camera(glm::vec3(1.0f, 1.0f, 5.0f));
    
    // Vegetation
    vegetation.push_back(glm::vec3(-1.5f,  0.0f, -0.48f));
    vegetation.push_back(glm::vec3( 1.5f,  0.0f,  0.51f));
    vegetation.push_back(glm::vec3( 0.0f,  0.0f,  0.7f));
    vegetation.push_back(glm::vec3(-0.3f,  0.0f, -2.3f));
    vegetation.push_back(glm::vec3( 0.5f,  0.0f, -0.6f));

    // cube VAO
    glGenVertexArrays(1, &cubeVAO);
    glGenBuffers(1, &cubeVBO);
    glBindVertexArray(cubeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), &cubeVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glBindVertexArray(0);
    // plane VAO
    glGenVertexArrays(1, &planeVAO);
    glGenBuffers(1, &planeVBO);
    glBindVertexArray(planeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, planeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(planeVertices), &planeVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glBindVertexArray(0);
    // grass
    glGenVertexArrays(1, &vegetationVAO);
    glGenBuffers(1, &vegetationVBO);
    glBindVertexArray(vegetationVAO);
    glBindBuffer(GL_ARRAY_BUFFER, vegetationVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(transparentVertices), &transparentVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glBindVertexArray(0);

    // load textures
    cubeTexture  = loadTexture("image/marble.jpg");
    floorTexture = loadTexture("image/metal.png");
    grassTexture = loadTexture("image/blending_transparent_window.png", GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);

    // shader
    shader->use();
    shader->setInt("texture1", 0);

    glFrontFace(GL_CW);
}

void drawStaff() {
    // Projection matrix
    glm::mat4 projection;
    projection = glm::perspective(glm::radians(camera->zoom), (float) screenWidth / screenHeight, 0.1f, 100.0f);
    glm::mat4 view = camera->getViewMatrix();
    glm::mat4 model = glm::mat4(1.0f);

    glEnable(GL_CULL_FACE);

    // Floor
    shader->use();
    shader->setMat4("view", view);
    shader->setMat4("projection", projection);
    shader->setMat4("model", glm::mat4(1.0f));
    glBindVertexArray(planeVAO);
    glBindTexture(GL_TEXTURE_2D, floorTexture);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    // cubes normal
    glBindVertexArray(cubeVAO);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, cubeTexture);
    model = glm::translate(model, glm::vec3(-1.0f, 0.0f, -1.0f));
    shader->setMat4("model", model);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(2.0f, 0.0f, 0.0f));
    shader->setMat4("model", model);
    glDrawArrays(GL_TRIANGLES, 0, 36);

    glDisable(GL_CULL_FACE);

    // Grass
    std::map<float, glm::vec3> sorted;
    for (int i = 0; i < vegetation.size(); i++) {
        float distance = glm::length(camera->position - vegetation[i]);
        sorted[distance] = vegetation[i];
    }

    glBindVertexArray(vegetationVAO);
    glBindTexture(GL_TEXTURE_2D, grassTexture);  
    for(std::map<float,glm::vec3>::reverse_iterator it = sorted.rbegin(); it != sorted.rend(); ++it) {
        model = glm::mat4(1.0f);
        model = glm::translate(model, it->second);               
        shader->setMat4("model", model);
        glDrawArrays(GL_TRIANGLES, 0, 6);
    }
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height);

void mouse_callback(GLFWwindow* window, double xpos, double ypos);

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

void processInput(GLFWwindow *window);

int main() {
    glfwInit();
    // OpenGL Version
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    // Using core profile
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    // For Mac OS X:
    // glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    // Create window object
    GLFWwindow* window = glfwCreateWindow(screenWidth, screenHeight, "LearnOpenGL", NULL, NULL);
    if (window == NULL) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    // Using GLAD to load OpenGL functions
    if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }
    // Define the Viewport
    glViewport(0, 0, screenWidth, screenHeight);
    // Register callbacks
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    // Capture the mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // Configuration
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);

    // Prepare for drawing
    prepareDraw();
    
    // Start render loop
    while (!glfwWindowShouldClose(window)) {
        // Input
        processInput(window);

        // Clear Screen
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Draw
        drawStaff();

        // Frame calc
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // Swap double buffer
        glfwSwapBuffers(window);
        // Deal with the events
        glfwPollEvents();    
    }

    delete shader;
    glfwTerminate();
    return 0;
}

/*
 * Callbacks
 */

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
    screenWidth = width;
    screenHeight = height;
}

bool mouseCap = true;
double lstChangeMouse = 0;

void processInput(GLFWwindow *window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }
    // Camera Pos
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera->processKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera->processKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera->processKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera->processKeyboard(RIGHT, deltaTime);
    // Release mouse
    if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS) {
        double now = glfwGetTime();
        if (now - lstChangeMouse > 0.2) {
            lstChangeMouse = now;
            mouseCap = !mouseCap;
            if (mouseCap) {
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            } else {
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            }
        } 
    }
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }
    
    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;
    lastX = xpos;
    lastY = ypos;

    if (mouseCap) {
        camera->processMouseMovement(xoffset, yoffset);
    }
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    camera->processMouseScroll(yoffset);
}