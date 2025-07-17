#include <iostream>
#include <random>
#include <string>

#define GLFW_INCLUDE_NONE
#include <glad/glad.h> 
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define IMGUI_IMPL_OPENGL_LOADER_GLAD
#include "imgui/imgui.h"
#include "imgui/imgui_impl_opengl3.h"
#include "imgui/imgui_impl_glfw.h"

#include "Simulation.h"

// Global variables
GLuint SCR_WIDTH = 1400;
GLuint SCR_HEIGHT = 900;
int N = 500; // Number of boids
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
GLuint VAO, meshVBO, instanceVBO, shaderProgram;

ImGuiIO* io = nullptr;
ImGuiStyle style;

// Mouse state tracking
bool leftMousePressed = false;
bool rightMousePressed = false;
bool middleMousePressed= false;

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
        // Style variables - Retain existing or slightly adjust for a softer sci-fi feel
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 10.0f); // Slightly more rounded windows
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 5.0f);   // More rounded interactive elements
        ImGui::PushStyleVar(ImGuiStyleVar_ScrollbarRounding, 5.0f); // Consistent rounding
        ImGui::PushStyleVar(ImGuiStyleVar_GrabRounding, 5.0f);     // Match frame rounding
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(15, 10)); // Increased padding for a cleaner look
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(8, 6));    // Slight increase in item spacing
        ImGui::PushStyleVar(ImGuiStyleVar_ItemInnerSpacing, ImVec2(6, 4)); //

        // Color palette - Shift towards sci-fi tones (blues, purples, greens, subtle desaturation)
        // Window background - Dark, desaturated blue/grey for a deep space feel
        ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.08f, 0.08f, 0.12f, 0.90f)); // Nearly opaque dark blue-grey
        ImGui::PushStyleColor(ImGuiCol_PopupBg, ImVec4(0.10f, 0.10f, 0.15f, 0.99f)); // Slightly lighter for popups

        // Borders - Subtle glowing effect or clean lines
        ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.25f, 0.40f, 0.55f, 0.70f)); // Muted blue border
        ImGui::PushStyleColor(ImGuiCol_BorderShadow, ImVec4(0.00f, 0.00f, 0.00f, 0.00f)); // No shadow for a flatter, digital look

        // Frames (e.g. input fields, checkboxes) - Clean, functional look
        ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.15f, 0.15f, 0.20f, 1.00f)); // Darker base for frames
        ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, ImVec4(0.20f, 0.25f, 0.35f, 0.70f)); // Subtle blue highlight on hover
        ImGui::PushStyleColor(ImGuiCol_FrameBgActive, ImVec4(0.25f, 0.30f, 0.45f, 0.90f)); // More pronounced active state

        // Buttons - Prominent, interactive elements with a distinct sci-fi glow
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.15f, 0.40f, 0.65f, 0.85f)); // Vibrant blue button
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.25f, 0.55f, 0.85f, 0.95f)); // Brighter, more saturated on hover
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.35f, 0.70f, 1.00f, 1.00f)); // Intense active glow

        // Sliders - Clear, functional with a glowing indicator
        ImGui::PushStyleColor(ImGuiCol_SliderGrab, ImVec4(0.20f, 0.50f, 0.75f, 0.90f)); // Distinct blue grab
        ImGui::PushStyleColor(ImGuiCol_SliderGrabActive, ImVec4(0.30f, 0.65f, 0.95f, 1.00f)); // Brighter when active

        // Headers (e.g. tree nodes, collapsing headers) - Subtle, organized sections
        ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.12f, 0.28f, 0.40f, 0.80f)); // Muted blue header
        ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.18f, 0.38f, 0.55f, 0.90f)); // Slightly brighter on hover
        ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImVec4(0.25f, 0.50f, 0.75f, 1.00f)); // More active blue

        // Scrollbar - Integrated and subtle
        ImGui::PushStyleColor(ImGuiCol_ScrollbarBg, ImVec4(0.06f, 0.06f, 0.09f, 0.60f)); // Darker scrollbar background
        ImGui::PushStyleColor(ImGuiCol_ScrollbarGrab, ImVec4(0.15f, 0.25f, 0.35f, 0.60f)); // Muted grab color
        ImGui::PushStyleColor(ImGuiCol_ScrollbarGrabHovered, ImVec4(0.20f, 0.30f, 0.45f, 0.80f)); // Slight highlight
        ImGui::PushStyleColor(ImGuiCol_ScrollbarGrabActive, ImVec4(0.25f, 0.40f, 0.55f, 1.00f)); // More prominent when active

        // Title Bar (e.g. window title bar when active) - Distinct, but not overly distracting
        ImGui::PushStyleColor(ImGuiCol_TitleBgActive, ImVec4(0.10f, 0.20f, 0.30f, 1.00f)); // Darker, distinct active title bar

        // Check Mark (for checkboxes and radio buttons) - Bright, clear indicator
        ImGui::PushStyleColor(ImGuiCol_CheckMark, ImVec4(0.20f, 0.70f, 0.30f, 1.00f)); // Bright green check mark

        // Text color - Standard white/light grey, but can be adjusted for a subtle blue tint
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.90f, 0.95f, 1.00f, 1.00f)); // Slightly blue-tinted white text
        ImGui::PushStyleColor(ImGuiCol_TextDisabled, ImVec4(0.40f, 0.45f, 0.50f, 1.00f)); // Muted text for disabled items

        // Separators - Subtle, yet visible
        ImGui::PushStyleColor(ImGuiCol_Separator, ImVec4(0.20f, 0.35f, 0.50f, 0.60f)); // Blue-tinted separator
        ImGui::PushStyleColor(ImGuiCol_SeparatorHovered, ImVec4(0.25f, 0.40f, 0.60f, 0.80f)); //
        ImGui::PushStyleColor(ImGuiCol_SeparatorActive, ImVec4(0.30f, 0.50f, 0.70f, 1.00f)); //

        // Plot colors (if you use ImGui::PlotLines or ImGui::PlotHistogram)
        ImGui::PushStyleColor(ImGuiCol_PlotLines, ImVec4(0.15f, 0.80f, 0.40f, 1.00f)); // Green for data plots
        ImGui::PushStyleColor(ImGuiCol_PlotLinesHovered, ImVec4(0.20f, 0.90f, 0.50f, 1.00f)); //
        ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(0.80f, 0.40f, 0.15f, 1.00f)); // Orange for histograms
        ImGui::PushStyleColor(ImGuiCol_PlotHistogramHovered, ImVec4(0.90f, 0.50f, 0.20f, 1.00f)); //

        // NavHighlight (for keyboard/gamepad navigation) - A clear focus indicator
        ImGui::PushStyleColor(ImGuiCol_NavHighlight, ImVec4(0.20f, 0.60f, 1.00f, 1.00f)); // Bright blue highlight
        ImGui::PushStyleColor(ImGuiCol_NavWindowingHighlight, ImVec4(0.70f, 0.70f, 0.70f, 0.70f)); //
        ImGui::PushStyleColor(ImGuiCol_NavWindowingDimBg, ImVec4(0.80f, 0.80f, 0.80f, 0.20f)); //
        ImGui::PushStyleColor(ImGuiCol_ModalWindowDimBg, ImVec4(0.20f, 0.20f, 0.20f, 0.35f)); //
    }
    return true;
}

void renderImgui(Simulation& sim)
{
    // Remove glfwPollEvents() from here - it should be in main loop
    // glfwPollEvents(); // REMOVE THIS LINE

    if (glfwGetWindowAttrib(window, GLFW_ICONIFIED) != 0)
    {
        ImGui_ImplGlfw_Sleep(10);
        return; // Add return to exit early when minimized
    }

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    // Fixed font scaling logic
    float FontScale = 1.0f;
    if (SCR_HEIGHT < 1920)
    {
        FontScale = 1.5f; // Adjust as needed
    }

    
    // Create the main UI window - REMOVED ImGuiWindowFlags_NoInputs
    ImGui::Begin("Boids Simulation v1.0", nullptr,
        ImGuiWindowFlags_NoCollapse |        // Prevent collapsing the window
        ImGuiWindowFlags_NoMove |            // Prevent moving the window
        ImGuiWindowFlags_NoResize |          // Prevent resizing the window
        ImGuiWindowFlags_NoTitleBar |        // Remove the default title bar for a custom look
        ImGuiWindowFlags_NoScrollbar |       // Remove scrollbar if content fits
        ImGuiWindowFlags_AlwaysAutoResize  // Automatically resize to fit content
    );

    ImGui::SetWindowPos(ImVec2(0,0), ImGuiCond_Always);
    ImGui::SetWindowSize(ImVec2(SCR_WIDTH / 3.0f, SCR_HEIGHT/2.25f), ImGuiCond_Always);
    ImGui::PushTextWrapPos();
    // Add actual UI elements for the boids simulation
    ImGui::Text("Boids Simulation Controls");
    ImGui::Separator();

    // Add simulation parameters (you'll need to add these to your Simulation class)

    ImGui::SliderFloat("Separation Weight", &sim.separation, 0.0f, 3.0f);
    ImGui::SliderFloat("Alignment Weight", &sim.alignment, 0.0f, 10.0f);
    ImGui::SliderFloat("Cohesion Weight", &sim.cohesion, 0.0f, 10.0f);
    ImGui::SliderFloat("Max Speed", &sim.maxSpeed, 0.001f, 1.5f);
    ImGui::SliderInt("FPS UPGRADE", &sim.friendUpdate, 1, 10);
    ImGui::Checkbox("Bounce of edges" ,&sim.bounce);
    ImGui::Checkbox("Friends making visualization" ,&sim.friendVisual);
    ImGui::Checkbox("Color based on speed" ,&sim.speedCol);

    ImGui::Separator();
    ImGui::Text("Mouse Controls:");
    ImGui::Text("Left Click: Attract boids");
    ImGui::Text("Right Click: Repel boids");

    ImGui::Separator();
    ImGui::Text("FPS: %d", FPS);
    ImGui::Text("Boids: %d", N);

    ImGui::End();

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
    glBufferData(GL_ARRAY_BUFFER, maxBufferSize * sizeof(BoidInstance), nullptr, GL_DYNAMIC_DRAW);

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
        boids[i].color = sim.Boids[i].visColor;
    }

    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, static_cast<int>(sim.Boids.size()) * sizeof(BoidInstance), boids);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void render() {

    float currentFrame = static_cast<float>(glfwGetTime());
    static float avgFPS = 0.0f;

    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;
    frame++;

    avgFPS += 1 / deltaTime;
    if (frame % 30 == 0) {
        FPS = avgFPS/30;
        avgFPS = 0.0f;


    }

    glClearColor(0.1f, 0.1f, 0.1f, 0.1f);
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(shaderProgram);
    glBindVertexArray(VAO);

    // Single instanced draw call for all boids
    glDrawArraysInstanced(GL_TRIANGLES, 0, 3, sim.Boids.size());
	renderImgui(sim);


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

    if (ImGui::GetCurrentContext() != nullptr) {

        if (!(*io).WantCaptureMouse) {



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


            if (button == GLFW_MOUSE_BUTTON_MIDDLE) {
                if (action == GLFW_PRESS) {
                    double xpos, ypos;
                    glfwGetCursorPos(window, &xpos, &ypos);
                    sim.mousePoint = ScreenToWorld(xpos, ypos);
                    std::cout << "Middle mouse pressed at: " << xpos << ", " << ypos << std::endl;
                    for (int i = 0; i < 100; i++) { sim.Boids.push_back(sim.generateBoid(sim.mousePoint)); }
                    N = static_cast<int>(sim.Boids.size());
                     
                    middleMousePressed = true;
                }
                else if(action == GLFW_RELEASE) {
                    middleMousePressed = false;
                }
            }

        }
    }

}

void cursor_position_callback(GLFWwindow* window, double xpos, double ypos) {


	if (leftMousePressed || rightMousePressed || middleMousePressed) {
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