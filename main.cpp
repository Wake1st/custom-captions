#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "imgui-filebrowser.h"

#include <ctime>
#include <iomanip>
#include <format>
#include <string>
#include <chrono>
#include <cstdlib>
#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "captioner.h"


void ErrorCallback(int error, const char* description)
{
	fprintf(stderr, "Error: %s\n", description);
}

std::string GetFormattedTimestamp() {
	// Get the current time point from the system clock
	auto now = std::chrono::system_clock::now();

	// Convert the time point to a time_t object
	std::time_t currentTime = std::chrono::system_clock::to_time_t(now);

	// Convert time_t to a tm struct for local time
	std::tm* localTime = std::localtime(&currentTime);

	// Create a string stream to format the time
	std::ostringstream oss;
	oss << std::put_time(localTime, "%Y-%m-%d_%H-%M-%S"); // Format string

	return oss.str();
}

std::string CreateWaveform(std::string path) {
	// create a standard file name thats always changing
	std::string filename = GetFormattedTimestamp();

	// command for creating the waveform
	std::string create_waveform_cmd = std::format(
		"ffmpeg -i {} -filter_complex \"[0:a]showwavespic=s=2560x2560,crop=2560:2560[v]\" -map \"[v]\" -update true -frames:v 1 ./data/{}_upr.png",
		path,
		filename
	);
	
	// command to rotate the waveform
	std::string rotate_waveform_cmd = std::format(
		"ffmpeg -i ./data/{}_upr.png -vf \"rotate=PI/2\" -update true -frames:v 1 ./data/{}.png",
		filename,
		filename
	);

	// execute commands
	int waveform_result = system(create_waveform_cmd.c_str());
	int rotation_result = system(rotate_waveform_cmd.c_str());

	return filename;
}

void DrawDockableParentWindow(bool *p_open, ImGui::FileBrowser *fileDialog) {
	static bool opt_fullscreen = true;
	static bool opt_padding = false;
	static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

	// We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
	// because it would be confusing to have two docking targets within each others.
	ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
	if (opt_fullscreen)
	{
		const ImGuiViewport* viewport = ImGui::GetMainViewport();
		ImGui::SetNextWindowPos(viewport->WorkPos);
		ImGui::SetNextWindowSize(viewport->WorkSize);
		ImGui::SetNextWindowViewport(viewport->ID);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
		window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
		window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
	}
	else
	{
		dockspace_flags &= ~ImGuiDockNodeFlags_PassthruCentralNode;
	}

	// When using ImGuiDockNodeFlags_PassthruCentralNode, DockSpace() will render our background
	// and handle the pass-thru hole, so we ask Begin() to not render a background.
	if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
		window_flags |= ImGuiWindowFlags_NoBackground;

	// Important: note that we proceed even if Begin() returns false (aka window is collapsed).
	// This is because we want to keep our DockSpace() active. If a DockSpace() is inactive,
	// all active windows docked into it will lose their parent and become undocked.
	// We cannot preserve the docking relationship between an active window and an inactive docking, otherwise
	// any change of dockspace/settings would lead to windows being stuck in limbo and never being visible.
	if (!opt_padding)
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
	ImGui::Begin("DockSpace Demo", p_open, window_flags);
	if (!opt_padding)
		ImGui::PopStyleVar();

	if (opt_fullscreen)
		ImGui::PopStyleVar(2);

	// Submit the DockSpace
	ImGuiIO& io = ImGui::GetIO();
	if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
	{
		ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
		ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
	}

	if (ImGui::BeginMenuBar())
    {
        if (ImGui::BeginMenu("Options"))
        {
            // Disabling fullscreen would allow the window to be moved to the front of other windows,
            // which we can't undo at the moment without finer window depth/z control.
            ImGui::MenuItem("Fullscreen", NULL, &opt_fullscreen);
            ImGui::Separator();

			if (ImGui::MenuItem("Open", "")) {
				fileDialog->Open();
			}

            ImGui::EndMenu();
        }

        ImGui::EndMenuBar();
    }

	ImGui::End();
}

int main() {
	// setup glfw
	glfwInit();
	glfwSetErrorCallback(ErrorCallback);

	// setup window
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* window = glfwCreateWindow(800, 800, "Custom Captions", NULL, NULL);
	if (window == NULL) {
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	// implement glad
	gladLoadGL();

	// setup imgui
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); //(void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Enable Docking
	io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;       // Enable Multi-Viewport / Platform Windows
	io.ConfigWindowsMoveFromTitleBarOnly = true;
	ImGui::StyleColorsDark();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 330");

	// run winow
	glViewport(0, 0, 1000, 800);

	// caption data
	Captioner* captioner = new Captioner();

	// create a file browser instance
	ImGui::FileBrowser fileDialog;

	// (optional) set browser properties
	fileDialog.SetTitle("File Browser");
	fileDialog.SetTypeFilters({ ".wav", ".mp3", ".ogg" });

	std::string filename = "EMPTY";

	while (!glfwWindowShouldClose(window))
	{
		// handle GLFW events
		glfwPollEvents();

		// set background color
		glClearColor(0.07f, 0.13f, 0.17f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		// set new frame
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		// set parent window
		DrawDockableParentWindow((bool*)true, &fileDialog);

		// tool
		captioner->update();

		// file browser
		fileDialog.Display();

		if (fileDialog.HasSelected())
		{
			// get path and clear dialog
			std::string path = fileDialog.GetSelected().string();
			fileDialog.ClearSelected();

			// use ffmpeg to create waveform of file
			filename = CreateWaveform(path);
			captioner->load(filename);
		}

		// render imgui
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		// Update and Render additional Platform Windows
		// (Platform functions may change the current OpenGL context, so we save/restore it to make it easier to paste this code elsewhere.
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			GLFWwindow* backup_current_context = glfwGetCurrentContext();
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
			glfwMakeContextCurrent(backup_current_context);
		}

		// swap buffers
		glfwSwapBuffers(window);
	}

	// close imgui
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	// close glfw window
	glfwDestroyWindow(window);

	// close glfw
	glfwTerminate();

	return 0;
}