#include <iostream>
#include <random>
#include <string>

#define GLFW_INCLUDE_NONE
#include <glad/glad.h> 
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "imgui/imgui.h"
#include "imgui/imgui_impl_win32.h"
#include "imgui/imgui_impl_opengl3.h"
#include "imgui/imgui_impl_glfw.h"

#include "Simulation.h"

// Global variables
GLuint SCR_WIDTH = 1200;
GLuint SCR_HEIGHT = 720;
const int N = 1000; // Number of boids
const int maxBufferSize = 10000; // max buffer size
int scale = 1.0f;

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

ImGuiIO* io = nullptr;
ImGuiStyle style;

// Mouse state tracking
bool leftMousePressed = false;
bool rightMousePressed = false;

const GLfloat boidMesh[] = {
    // Triangle vertices (local space, pointing right)
     0.02f,  0.0f,    // tip
    -0.01f,  0.008f,  // back top
    -0.01f, -0.008f,   // back bottom
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
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void cursor_position_callback(GLFWwindow* window, double xpos, double ypos);
glm::vec2 ScreenToWorld(double xpos, double ypos);


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
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetCursorPosCallback(window, cursor_position_callback);

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

bool initializeImGUI() {

#if defined(IMGUI_IMPL_OPENGL_ES2)
    // GL ES 2.0 + GLSL 100
    const char* glsl_version = "#version 100";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
#elif defined(__APPLE__)
    // GL 3.2 + GLSL 150
    const char* glsl_version = "#version 150";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // Required on Mac
#else
    // GL 3.0 + GLSL 130
    const char* glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    //glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // 3.0+ only
#endif

 //imgui initialization
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    io = &ImGui::GetIO(); (void)io;
    io->ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io->ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);

#ifdef __EMSCRIPTEN__
    ImGui_ImplGlfw_InstallEmscriptenCallbacks(window, "#canvas");
#endif
    ImGui_ImplOpenGL3_Init(glsl_version);
    style = ImGui::GetStyle();

    //IMGUI STYLE
    {
        // Style variables
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 8.0f);        // Slightly larger rounding for modern look
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 3.0f);         // Consistent rounded elements
        ImGui::PushStyleVar(ImGuiStyleVar_ScrollbarRounding, 4.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_GrabRounding, 4.0f);          // Matched with frame rounding
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(12, 8));// More balanced paddin

        // Color palette
       // Window background
        ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.12f, 0.12f, 0.16f, 0.95f));  // Nearly opaque with just a hint of transparency
        ImGui::PushStyleColor(ImGuiCol_PopupBg, ImVec4(0.14f, 0.14f, 0.18f, 0.99f));  // Popups nearly as opaque

        // Borders
        ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.30f, 0.30f, 0.40f, 0.60f));  // Subtle, slightly transparent border
        ImGui::PushStyleColor(ImGuiCol_BorderShadow, ImVec4(0.00f, 0.00f, 0.00f, 0.30f));  // Soft shadow for depth

        // Frames (e.g. input fields, checkboxes)
        ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.20f, 0.20f, 0.25f, 1.00f));  // Solid background for frames
        ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, ImVec4(0.30f, 0.25f, 0.45f, 0.60f));  // Slightly transparent on hover
        ImGui::PushStyleColor(ImGuiCol_FrameBgActive, ImVec4(0.40f, 0.35f, 0.60f, 0.80f));  // Active state with increased opacity for clarity

        // Buttons
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.40f, 0.45f, 0.80f, 0.80f));  // Modern saturated look
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.50f, 0.55f, 0.95f, 0.90f));  // Brighter on hover
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.60f, 0.50f, 1.00f, 1.00f));  // Fully saturated when active

        // Sliders
        ImGui::PushStyleColor(ImGuiCol_SliderGrab, ImVec4(0.50f, 0.55f, 0.90f, 0.80f));  // Clear slider grab
        ImGui::PushStyleColor(ImGuiCol_SliderGrabActive, ImVec4(0.70f, 0.60f, 1.00f, 1.00f));  // More vivid when active

        // Headers (e.g. tree nodes, collapsing headers)
        ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.35f, 0.40f, 0.70f, 0.80f));  // Subdued, balanced header color
        ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.45f, 0.50f, 0.85f, 0.90f));  // Slightly brighter on hover
        ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImVec4(0.60f, 0.50f, 1.00f, 1.00f));  // Fully saturated when active

        // Scrollbar
        ImGui::PushStyleColor(ImGuiCol_ScrollbarBg, ImVec4(0.10f, 0.10f, 0.12f, 0.60f));  // Background for scrollbars
        ImGui::PushStyleColor(ImGuiCol_ScrollbarGrab, ImVec4(0.30f, 0.30f, 0.40f, 0.60f));  // Grab area for scrollbars
        ImGui::PushStyleColor(ImGuiCol_ScrollbarGrabHovered, ImVec4(0.40f, 0.40f, 0.50f, 0.80f));  // Lighter when hovered
        ImGui::PushStyleColor(ImGuiCol_ScrollbarGrabActive, ImVec4(0.50f, 0.50f, 0.60f, 1.00f));  // Distinct active state

        // Title Bar (e.g. window title bar when active)
        ImGui::PushStyleColor(ImGuiCol_TitleBgActive, ImVec4(0.18f, 0.18f, 0.24f, 1.00f));  // Strong visual identity for active title bars

        // Check Mark (for checkboxes and radio buttons)
        ImGui::PushStyleColor(ImGuiCol_CheckMark, ImVec4(0.70f, 0.65f, 1.00f, 1.00f));  // Brighter and more visible check marks
        // If you have additional custom styling, add them here…
    }
    return true;
}

void renderImgui(Simulation &sim)
{

    glfwPollEvents();
    if (glfwGetWindowAttrib(window, GLFW_ICONIFIED) != 0)
    {
        ImGui_ImplGlfw_Sleep(10);
        //continue;
    }
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    //if (show_demo_window)
   // ImGui::ShowDemoWindow();

    
        static float f = 0.0f;
        static int counter = 0;

		ImGui::Begin("StereoShape v1.2", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar);
         //   ImGui::Begin("StereoShape v1.2", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar);

        ImGui::SetWindowPos(ImVec2(0, 0));
        //float FontScale = float(SCR_HEIGHT) / 800 / 1.5;

        if (SCR_HEIGHT < 1920)
        {
            float FontScale = 2;
        }
        float FontScale = 1;


        //static float userFontScale = 1.198f;
        static float userFontScale = 1.0f;

        ImGui::SetWindowSize(ImVec2(SCR_WIDTH / 5.4 * userFontScale * 1.1, SCR_HEIGHT));

        ImGui::GetIO().FontGlobalScale = FontScale * userFontScale;

        // Display some text (you can use a format strings too)

//ImGui::Checkbox("Another Window", &show_another_window);
//ImGui::SliderInt("renderDistance", &renderDistance, 0, 16);
// ImGui::SliderInt("world gen distance", &worldGenDistance, 0, 16);


    ImGui::Render();

    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());


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
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_COLOR);
  //  glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
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
    glDrawArraysInstanced(GL_TRIANGLES, 0, 6, N);

    glfwSwapBuffers(window);
}

int main() {
    if (!initializeOpenGL()) return -1;
    if (!initializeImGUI()) return -1;
    if (!createShaders()) return -1;
    setupBuffers();

    updateInstanceBuffer();


    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);

        sim.update(deltaTime);
        updateInstanceBuffer();
        render();
        renderImgui(sim);
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

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        if (action == GLFW_PRESS) {
            leftMousePressed = true;
            // Update mouse position immediately when pressed
            double xpos, ypos;
            glfwGetCursorPos(window, &xpos, &ypos);
            sim.mousePoint = ScreenToWorld(xpos, ypos);
            sim.atract = true;
        }
        else if (action == GLFW_RELEASE) {
            leftMousePressed = false;
            sim.atract = false;
        }
    }

    if (button == GLFW_MOUSE_BUTTON_RIGHT) {
		if (action == GLFW_PRESS) {
				rightMousePressed = true;
				double xpos, ypos;
				glfwGetCursorPos(window, &xpos, &ypos);
				sim.mousePoint = ScreenToWorld(xpos, ypos);
				sim.repel = true;
		}
		else if (action == GLFW_RELEASE) {
				rightMousePressed = false;
				sim.repel = false;
		}
    }

}

void cursor_position_callback(GLFWwindow* window, double xpos, double ypos) {
    // Only update mouse position if left mouse button is pressed
    if (leftMousePressed || rightMousePressed) {
        sim.mousePoint = ScreenToWorld(xpos, ypos);
    }
}

glm::vec2 ScreenToWorld(double xpos, double ypos) {

    float xNDC = (2.0f * xpos) / SCR_WIDTH - 1.0f;
    float yNDC = 1.0f - (2.0f * ypos) / SCR_HEIGHT;
    float worldX = xNDC * aspect;
    float worldY = yNDC;

    return glm::vec2(worldX, worldY);

}

void cleanup() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    delete[] boids;
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &meshVBO);
    glDeleteBuffers(1, &instanceVBO);
    glDeleteProgram(shaderProgram);
    glfwDestroyWindow(window);
    glfwTerminate();
}