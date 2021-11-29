#include "Simulation.h"
#include "Camera.h"
#include "Renderer.h"


// Initialise this as well as the renderer, returns false on error
bool Simulation::Initialise()
{
	// Set up camera
	m_camera = std::make_shared<Helpers::Camera>();
	//m_camera->Initialise(glm::vec3(0, 200, 900), glm::vec3(0)); // Jeep
	//m_camera->Initialise(glm::vec3(-13.82f, 5.0f, 1.886f), glm::vec3(0.25f, 1.5f, 0), 30.0f,0.8f); // Aqua pig
	m_camera->Initialise(glm::vec3(0, 20, 60), glm::vec3(0.3f, 0, 0)); // Cube

	// Set up renderer
	m_renderer = std::make_shared<Renderer>();
	return m_renderer->InitialiseGeometry();
}

// Handle any user input. Return false if program should close.
bool Simulation::HandleInput(GLFWwindow* window)
{	
	// Not if it is being handled by IMGUI
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	if (io.WantCaptureKeyboard || io.WantCaptureMouse)
		return true;

	// You can get keypresses like this:
	// if (glfwGetKey(window, GLFW_KEY_W)==GLFW_PRESS) // means W key pressed

	// You can get mouse button input, returned state is GLFW_PRESS or GLFW_RELEASE
	// int state = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);

	// Use this to get the mouse position:
	// double xpos, ypos;
	// glfwGetCursorPos(window, &xpos, &ypos);

	// To prevent the mouse leaving the window (and to hide the arrow) you can call:
	// glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	// To reenable it use GLFW_CURSOR_NORMAL

	// To see an example of input using GLFW see the camera.cpp file.
	
	return true;
}

// Update the simulation (and render) returns false if program should close
bool Simulation::Update(GLFWwindow* window)
{
	// Deal with any input
	if (!HandleInput(window))
		return false;

	// Calculate delta time since last called
	// We pass the delta time to the camera and renderer
	float timeNow = (float)glfwGetTime();
	float deltaTime{ timeNow - m_lastTime };
	m_lastTime = timeNow;

	// The camera needs updating to handle user input internally
	m_camera->Update(window, deltaTime);

	// Render the scene
	m_renderer->Render(*m_camera, deltaTime);

	// IMGUI	
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
	m_renderer->DefineGUI();

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

	return true;
}
