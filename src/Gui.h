#define IMGUI_IMPL_OPENGL_LOADER_GLAD
#include "imgui/imgui.h"
#include "imgui/imgui_impl_opengl3.h"
#include "imgui/imgui_impl_glfw.h"

// Forward declarations for external variables from main.cpp
extern GLFWwindow* window;
extern GLuint SCR_WIDTH;
extern GLuint SCR_HEIGHT;
extern int spawnCount;
extern bool spawnPredators;
extern int FPS;
extern int N;
extern float scale;

class GUI {

public:
	ImGuiIO* io = nullptr;
	ImGuiStyle style;

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
			ImGuiWindowFlags_NoScrollbar |       // Remove scrollbar if content fits
			ImGuiWindowFlags_AlwaysAutoResize |  // Automatically resize to fit content
			ImGuiWindowFlags_NoMove
		);

		ImGui::SetWindowPos(ImVec2(0, 0), ImGuiCond_Always);
		ImGui::SetWindowSize(ImVec2(SCR_WIDTH / 3.0f, SCR_HEIGHT / 2.25f), ImGuiCond_Always);
		ImGui::PushTextWrapPos();
		// Add actual UI elements for the boids simulation
		ImGui::Text("Boids Simulation Controls");
		ImGui::Separator();

		// Add simulation parameters (you'll need to add these to your Simulation class)

		ImGui::SliderFloat("Separation Weight", &sim.separation, 0.0f, 3.0f);
		ImGui::SliderFloat("Alignment Weight", &sim.alignment, 0.0f, 10.0f);
		ImGui::SliderFloat("Cohesion Weight", &sim.cohesion, 0.0f, 10.0f);
		ImGui::SliderFloat("Max Speed", &sim.maxSpeed, 0.001f, 1.5f);
		ImGui::SliderFloat("Min Speed", &sim.minSpeed, 0.001f, 1.5f);
		ImGui::SliderFloat("FOV range", &sim.fovRadius, 0.0f, 1.0f);
		ImGui::SliderFloat("Scale", &scale, 0.001f, 3.0f);
		ImGui::Checkbox("Bounce of edges", &sim.bounce);
		ImGui::Checkbox("Friends making visualization", &sim.friendVisual);
		ImGui::Checkbox("Color based on speed", &sim.speedCol);
		ImGui::SliderInt("Spawning count", &spawnCount, 1, 20);
		ImGui::Checkbox("Spawn predators", &spawnPredators);

		ImGui::Separator();
		ImGui::Text("Mouse Controls:");
		ImGui::Text("Left Click: Attract boids");
		ImGui::Text("Right Click: Repel boids");
		ImGui::Text("Middle Click: Generate boids");

		ImGui::Separator();
		ImGui::Text("FPS: %d", FPS);
		ImGui::Text("Boids: %d", N);

		ImGui::End();

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	}
};

