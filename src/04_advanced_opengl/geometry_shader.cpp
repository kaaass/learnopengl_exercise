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
#include "../common_draw.h"

int screenWidth = 1280;
int screenHeight = 720;

const GLuint SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;

Shader* shader = nullptr;
Shader* normalDisplayShader = nullptr;

Model* model = nullptr;

float deltaTime = 0.0f;
float lastFrame = 0.0f;

Camera* camera = nullptr;
float lastX = screenWidth / 2;
float lastY = screenHeight / 2;
bool firstMouse = true;

void prepareDraw() {
    // Create shader
    shader = new Shader("shader/geometry_shader.vs",
                        "shader/geometry_shader.fs");
    normalDisplayShader = 
            new Shader("shader/normalDisplayShader.vs",
                        "shader/normalDisplayShader.gs",
                        "shader/normalDisplayShader.fs");
    // Create camera
    camera = new Camera(glm::vec3(1.0f, 1.0f, 5.0f));

    model = new Model("../03_model_loading/model/nanosuit/nanosuit.obj");
}

void drawStaff() {
    shader->use();
    shader->setFloat("time", glfwGetTime());

    glm::mat4 projection = glm::perspective(glm::radians(camera->zoom), (float) screenWidth / screenHeight, 0.1f, 100.0f);
    glm::mat4 view = camera->getViewMatrix();
    glm::mat4 modelMat = glm::mat4(1.0f);
    shader->setMat4("projection", projection);
    shader->setMat4("view", view);
    shader->setMat4("model", modelMat);

    model->draw(*shader);

    normalDisplayShader->use();
    normalDisplayShader->setMat4("projection", projection);
    normalDisplayShader->setMat4("view", view);
    normalDisplayShader->setMat4("model", modelMat);

    model->draw(*normalDisplayShader);
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
    glFrontFace(GL_CW);

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
bool capKeyPressed = false;

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
    if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS && !capKeyPressed) {
        mouseCap = !mouseCap;
        if (mouseCap) {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        } else {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }
        capKeyPressed = true;
    }
    if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_RELEASE) {
        capKeyPressed = false;
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