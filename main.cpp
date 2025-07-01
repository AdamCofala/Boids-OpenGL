#include <iostream>
#include <random>
#define GLFW_INCLUDE_NONE
#include <glad/glad.h> 
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <string>
#include "Simulation.h"

// Global variables
GLuint SCR_WIDTH = 1200;
GLuint SCR_HEIGHT = 720;
const int N = 150; // Number of boids
const int maxBufferSize = 10000; // max buffer size
int scale = 2.0f;

float aspect = (float)SCR_WIDTH / (float)SCR_HEIGHT;
int   frame = 0;
int   FPS = 0;
float deltaTime = 0.0f;
float lastFrame = 0.0f;

Simulation sim(N, aspect);

// OpenGL objects
GLFWwindow* window = nullptr;
std::string windowTitle = "Boids, FPS: ";
GLuint VAO, meshVBO, instanceVBO, shaderProgram;

const GLfloat boidMesh[] = {
    // Triangle vertices (local space, pointing right)
     0.02f,  0.0f,    // tip
    -0.01f,  0.008f,  // back top
    -0.01f, -0.008f   // back bottom
};

struct BoidInstance {
    glm::vec2 position;
    float rotation;     // angle in radians
    glm::vec3 color;
    float scale;
};

BoidInstance* boids = new BoidInstance[N];
const char* vertexShaderSource = R"(#version 330 core
layout (location = 0) in vec2 aLocalPos;
layout (location = 1) in vec2 aInstancePos;
layout (location = 2) in float aRotation;
layout (location = 3) in vec3 aColor;
layout (location = 4) in float aScale;

out vec3 Color;
uniform mat4 projection;

void main() {
    float c = cos(aRotation);
    float s = sin(aRotation);
    mat2 rotMatrix = mat2(c, -s, s, c);

    vec2 scaledPos = aLocalPos * aScale;
    vec2 worldPos = rotMatrix * scaledPos + aInstancePos;

    gl_Position = projection * vec4(worldPos, 0.0, 1.0);
    Color = aColor;
})";

const char* fragmentShaderSource = R"(#version 330 core
in vec3 Color;
out vec4 FragColor;

void main() {
    FragColor = vec4(Color, 1.0);
})";

// Function prototypes
bool initializeOpenGL();
bool createShaders();
void setupBuffers();
void updateInstanceBuffer();
void render();
void cleanup();
void framebuffer_size_callback(GLFWwindow* window, int width, int height);

bool initializeOpenGL() {
    // Initialize GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return false;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, 8); // 8x MSAA

    // Create window
    window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Instanced Flocking Simulation 2D", NULL, NULL);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return false;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // Initialize GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return false;
    }

    glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
    glEnable(GL_MULTISAMPLE);
    glDisable(GL_DEPTH_TEST);

    return true;
}

bool createShaders() {
    // Vertex shader
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);

    // Check vertex shader compilation
    GLint success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cerr << "Vertex shader compilation failed: " << infoLog << std::endl;
        return false;
    }

    // Fragment shader
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);

    // Check fragment shader compilation
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cerr << "Fragment shader compilation failed: " << infoLog << std::endl;
        return false;
    }

    // Create shader program
    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    // Check program linking
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cerr << "Shader program linking failed: " << infoLog << std::endl;
        return false;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // Set projection matrix
    float aspect = (float)SCR_WIDTH / (float)SCR_HEIGHT;
    glm::mat4 projection = glm::ortho(-aspect, aspect, -1.0f, 1.0f, -1.0f, 1.0f);

    glUseProgram(shaderProgram);
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

    return true;
}

void setupBuffers() {
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &meshVBO);
    glGenBuffers(1, &instanceVBO);

    glBindVertexArray(VAO);

    // Set up mesh buffer (local vertices)
    glBindBuffer(GL_ARRAY_BUFFER, meshVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(boidMesh), boidMesh, GL_DYNAMIC_DRAW);

    // Vertex attribute 0: Local position (2D)
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Set up instance buffer
    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
    glBufferData(GL_ARRAY_BUFFER, maxBufferSize * sizeof(BoidInstance), nullptr, GL_STREAM_DRAW);

    // Instance attribute 1: Position (2D)
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(BoidInstance), (void*)offsetof(BoidInstance, position));
    glEnableVertexAttribArray(1);
    glVertexAttribDivisor(1, 1);

    // Instance attribute 2: Rotation (1 float)
    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(BoidInstance), (void*)offsetof(BoidInstance, rotation));
    glEnableVertexAttribArray(2);
    glVertexAttribDivisor(2, 1);

    // Instance attribute 3: Color (3 floats)
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(BoidInstance), (void*)offsetof(BoidInstance, color));
    glEnableVertexAttribArray(3);
    glVertexAttribDivisor(3, 1);

    // Instance attribute 4: Scale (1 float)
    glVertexAttribPointer(4, 1, GL_FLOAT, GL_FALSE, sizeof(BoidInstance), (void*)offsetof(BoidInstance, scale));
    glEnableVertexAttribArray(4);
    glVertexAttribDivisor(4, 1);

    // Cleanup
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // Optional render states (place these in render setup if possible)
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);
}

void updateInstanceBuffer() {

    for (int i = 0; i < sim.Boids.size(); i++) {
         boids[i].position = sim.Boids[i].pos;

         boids[i].scale = scale;
         boids[i].rotation = sim.Boids[i].getRotation();
         boids[i].color = sim.Boids[i].color;
    }

    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, N * sizeof(BoidInstance), boids);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void render() {

    float currentFrame = static_cast<float>(glfwGetTime());
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;
    frame++;
    if (frame % 5 == 0) {
        FPS = (1 / deltaTime);
        glfwSetWindowTitle(window, (windowTitle + std::to_string(FPS)).c_str());
    }

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(shaderProgram);
    glBindVertexArray(VAO);

    // Single instanced draw call for all boids
    glDrawArraysInstanced(GL_TRIANGLES, 0, 3, N);

    glfwSwapBuffers(window);
}

int main() {
    if (!initializeOpenGL()) return -1;
    if (!createShaders()) return -1;
    setupBuffers();

    // Main loop

     updateInstanceBuffer();

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);
        
        sim.update();
        updateInstanceBuffer();
        render();
    }

    cleanup();
    return 0;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    SCR_WIDTH = width;
    SCR_HEIGHT = height;
    glViewport(0, 0, width, height);

    // Update projection matrix
    aspect = float(width) / float(height);
    glm::mat4 projection = glm::ortho(-aspect, aspect, -1.0f, 1.0f, -1.0f, 1.0f);
    sim.updateAspect(aspect);

    glUseProgram(shaderProgram);
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
}

void cleanup() {
    delete[] boids;
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &meshVBO);
    glDeleteBuffers(1, &instanceVBO);
    glDeleteProgram(shaderProgram);
    glfwDestroyWindow(window);
    glfwTerminate();
}