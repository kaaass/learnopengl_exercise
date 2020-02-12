#include <iostream>
#include <cmath>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "../shader_s.h"
#define STB_IMAGE_IMPLEMENTATION
#include "../stb_image.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "vertex_data_textures.h"
#include "../camera.h"

const char* vertShaderPath = "shader/light_casters.vs";
const char* fragShaderPath = "shader/light_casters.direction.fs";
const char* lampVertShaderPath = "shader/colors.vs";
const char* lampFragShaderPath = "shader/colors_light.fs";

int screenWidth = 800;
int screenHeight = 600;

Shader* lightingShader = nullptr;
Shader* lampShader = nullptr;

unsigned int VBO;
unsigned int VAO;
unsigned int lightVAO;
unsigned int diffuseMap;
unsigned int specularMap;

float deltaTime = 0.0f;
float lastFrame = 0.0f;

glm::vec3 lightPos(1.2f, 1.0f, 2.0f);

Camera* camera = nullptr;
float lastX = screenWidth / 2;
float lastY = screenHeight / 2;
bool firstMouse = true;

glm::vec3 cubePositions[] = {
    glm::vec3( 0.0f,  0.0f,  0.0f),
    glm::vec3( 2.0f,  5.0f, -15.0f),
    glm::vec3(-1.5f, -2.2f, -2.5f),
    glm::vec3(-3.8f, -2.0f, -12.3f),
    glm::vec3( 2.4f, -0.4f, -3.5f),
    glm::vec3(-1.7f,  3.0f, -7.5f),
    glm::vec3( 1.3f, -2.0f, -2.5f),
    glm::vec3( 1.5f,  2.0f, -2.5f),
    glm::vec3( 1.5f,  0.2f, -1.5f),
    glm::vec3(-1.3f,  1.0f, -1.5f)
};

int loadTexture(char * filepath, int mode = GL_RGB) {
    unsigned int texture;
    std::cout << "Start loading texture: " << filepath << std::endl;
    // Create texture
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    // Configure wrap and filter type
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // Load and generate texture
    int width, height, nrChannels;
    stbi_set_flip_vertically_on_load(true);
    unsigned char *data = stbi_load(filepath, &width, &height, &nrChannels, 0);
    if (data) {
        glTexImage2D(GL_TEXTURE_2D, 0, mode, width, height, 0, mode, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    } else {
        std::cout << "Failed to load texture" << std::endl;
    }
    // Free image
    stbi_image_free(data);
    return texture;
}

void prepareDraw() {
    // Load texture
    diffuseMap = loadTexture("image/container2.png", GL_RGBA);
    specularMap = loadTexture("image/container2_specular.png", GL_RGBA);

    // Create camera
    camera = new Camera(glm::vec3(1.0f, 1.0f, 5.0f));

    // Create VBO, VAO
    glGenBuffers(1, &VBO);
    glGenVertexArrays(1, &VAO);
    glGenVertexArrays(1, &lightVAO);

    // Rectangle
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    // Pos
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // Normal
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    // Texture
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);
    
    glBindVertexArray(lightVAO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
}

void drawStaff() {
    lightingShader->use();

    // Projection matrix
    glm::mat4 projection;
    projection = glm::perspective(glm::radians(camera->zoom), (float) screenWidth / screenHeight, 0.1f, 100.0f);

    // Pass the transform matrixs
    lightingShader->setMat4("view", camera->getViewMatrix());
    lightingShader->setMat4("projection", projection);

    // Lighting
    lightingShader->setVec3("viewPos", camera->position);
    lightingShader->setVec3("light.ambient",  0.2f, 0.2f, 0.2f);
    lightingShader->setVec3("light.diffuse",  0.5f, 0.5f, 0.5f);
    lightingShader->setVec3("light.specular", 1.0f, 1.0f, 1.0f);
    lightingShader->setVec3("light.position", lightPos);
    lightingShader->setVec3("light.direction", -0.2f, -1.0f, -0.3f);
    lightingShader->setInt("material.diffuse", 0);
    lightingShader->setVec3("material.specular", 0.5f, 0.5f, 0.5f);
    lightingShader->setFloat("material.shininess", 32.0f);
    lightingShader->setInt("material.specular", 1);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, diffuseMap);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, specularMap);
        
    // Drawing
    glBindVertexArray(VAO);
    glm::mat4 model;    
    for (int i = 0; i < 10; i++) {
        glm::mat4 model;
        model = glm::translate(model, cubePositions[i]);
        float angle = 20.0f * i;
        model = glm::rotate(model, glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));
        lightingShader->setMat4("model", model);

        glDrawArrays(GL_TRIANGLES, 0, 36);
    }

    // Lamp cube
    // lampShader->use();

    // model = glm::mat4();
    // model = glm::translate(model, lightPos);
    // model = glm::scale(model, glm::vec3(0.2f));
    // lampShader->setMat4("model", model);
    // lampShader->setMat4("view", camera->getViewMatrix());
    // lampShader->setMat4("projection", projection);

    // glBindVertexArray(lightVAO);
    // glDrawArrays(GL_TRIANGLES, 0, 36);
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
    glViewport(0, 0, 800, 600);
    // Register callbacks
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    // Capture the mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // Configuration
    glEnable(GL_DEPTH_TEST);

    // Prepare for drawing
    lightingShader = new Shader(vertShaderPath, fragShaderPath);
    lampShader = new Shader(lampVertShaderPath, lampFragShaderPath);
    prepareDraw();
    
    // Start render loop
    while (!glfwWindowShouldClose(window)) {
        // Input
        processInput(window);

        // Clear Screen
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
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

    delete lightingShader;
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

    camera->processMouseMovement(xoffset, yoffset);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    camera->processMouseScroll(yoffset);
}