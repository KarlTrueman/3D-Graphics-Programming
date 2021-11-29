#include "Helper.h"

#include <fstream>
#include <sstream>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <stdio.h>

namespace Helpers
{
	// OpenGL Error Callback
	void APIENTRY glDebugOutput(GLenum source,
		GLenum type,
		unsigned int id,
		GLenum severity,
		GLsizei length,
		const char* message,
		const void* userParam)
	{
		// ignore non-significant error/warning codes

		// 131185 - says when a static buffer will use video memory, just info so ignoring
		if (id == 131185)
			return;

		// This is just info for when frame buffer storage is allocated, so ignore
		if (id == 131169)
			return;

		// Program/shader state performance warning. Lots of these happen so ignoring.
		if (id == 131218)
			return;

		// The texture object (0) bound to texture image unit 0 does not have a defined base level and cannot be used for texture mapping.
		// Which is nonsense so ignore
		if (id == 131204)
			return;

		std::cout << "---------------" << std::endl;
		std::cout << "OpenGL Debug message (" << id << "): " << message << std::endl;

		switch (source)
		{
			case GL_DEBUG_SOURCE_API:             std::cout << "Source: API"; break;
			case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   std::cout << "Source: Window System"; break;
			case GL_DEBUG_SOURCE_SHADER_COMPILER: std::cout << "Source: Shader Compiler"; break;
			case GL_DEBUG_SOURCE_THIRD_PARTY:     std::cout << "Source: Third Party"; break;
			case GL_DEBUG_SOURCE_APPLICATION:     std::cout << "Source: Application"; break;
			case GL_DEBUG_SOURCE_OTHER:           std::cout << "Source: Other"; break;
		} 
		std::cout << std::endl;

		switch (type)
		{
			case GL_DEBUG_TYPE_ERROR:               std::cout << "Type: Error"; break;
			case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: std::cout << "Type: Deprecated Behaviour"; break;
			case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  std::cout << "Type: Undefined Behaviour"; break;
			case GL_DEBUG_TYPE_PORTABILITY:         std::cout << "Type: Portability"; break;
			case GL_DEBUG_TYPE_PERFORMANCE:         std::cout << "Type: Performance"; break;
			case GL_DEBUG_TYPE_MARKER:              std::cout << "Type: Marker"; break;
			case GL_DEBUG_TYPE_PUSH_GROUP:          std::cout << "Type: Push Group"; break;
			case GL_DEBUG_TYPE_POP_GROUP:           std::cout << "Type: Pop Group"; break;
			case GL_DEBUG_TYPE_OTHER:               std::cout << "Type: Other"; break;
		} 
		std::cout << std::endl;

		switch (severity)
		{
			case GL_DEBUG_SEVERITY_HIGH:         std::cout << "Severity: high"; break;
			case GL_DEBUG_SEVERITY_MEDIUM:       std::cout << "Severity: medium"; break;
			case GL_DEBUG_SEVERITY_LOW:          std::cout << "Severity: low"; break;
			case GL_DEBUG_SEVERITY_NOTIFICATION: std::cout << "Severity: notification"; break;
		} 
		std::cout << std::endl;
		std::cout << std::endl;
	}

	// Uses GLFW to set up a window via GLFW. Also initialises GLEW and OpenGL.
	GLFWwindow* CreateGLFWWindow(int width, int height, const std::string& title)
	{
		if (!glfwInit())
		{
			std::cout << "Failed to initialise GLFW" << std::endl;
			return nullptr;
		}

		std::cout << "GLFW initialised" << std::endl;

		glfwWindowHint(GLFW_RED_BITS, 8);
		glfwWindowHint(GLFW_GREEN_BITS, 8);
		glfwWindowHint(GLFW_BLUE_BITS, 8);
		glfwWindowHint(GLFW_ALPHA_BITS, 0);
		glfwWindowHint(GLFW_DEPTH_BITS, 24);
		glfwWindowHint(GLFW_STENCIL_BITS, 8);
		glfwWindowHint(GLFW_SAMPLES, 4); // 4x antialiasing, could raise e.g. to 8
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4); // We want OpenGL 4.6 minimum
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
		const char* glsl_version = "#version 460";	// For imgui

		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // We don't want the old OpenGL 

		glEnable(GL_MULTISAMPLE);

// Turn on debug mode
#ifdef _DEBUG
		glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
#endif

		GLFWwindow* window{ glfwCreateWindow(width, height, title.c_str(), NULL, NULL) };
		if (!window)
		{
			std::cout << "Failed to create window" << std::endl;
			glfwTerminate();
			return nullptr;
		}

		glfwMakeContextCurrent(window);
		glewExperimental = true; // Needed in core profile

		std::cout << "GLFW window initialised" << std::endl;

		GLenum err{ glewInit() };
		if (GLEW_OK != err)
		{			
			auto errs = glewGetErrorString(err);			

			std::cout << "Failed to initialise GLEW. Error" << errs << std::endl;

			return nullptr;
		}

// Enable trapping of OpenGL errors via a callback
#if defined(_DEBUG)			
		int flags{ 0 };
		glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
		if (flags & GL_CONTEXT_FLAG_DEBUG_BIT)
		{
			// initialize debug output 
			glEnable(GL_DEBUG_OUTPUT);
			glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
			glDebugMessageCallback(glDebugOutput, nullptr);
			glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
		}
#endif

		std::cout << "GLEW initialised" << std::endl;

		// The framebuffer size needs to be retrieved for glViewport.
		int fbwidth, fbheight;
		glfwGetFramebufferSize(window, &fbwidth, &fbheight);
		glViewport(0, 0, fbwidth, fbheight);

		glfwSwapInterval(0);

		// Also set up IMGUI while here
		// Setup Dear ImGui context
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO(); (void)io;
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
		//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

		// Setup Dear ImGui style
		ImGui::StyleColorsDark();
		ImGui_ImplGlfw_InitForOpenGL(window, true);
		ImGui_ImplOpenGL3_Init(glsl_version);

		return window;
	}

	// Loads a whole file into a string e.g. for shaders
	std::string stringFromFile(const std::string& filepath)
	{
		std::ifstream fp;
		fp.open(filepath, std::ifstream::in);
		if (fp.is_open() == false) {
			return "";
		}
		std::stringstream ss;
		ss << fp.rdbuf();
		return ss.str();
	}

	// Check shader with id compiled without error
	bool DidShaderCompileOK(GLuint id)
	{
		GLint compileStatus{ 0 };
		glGetShaderiv(id, GL_COMPILE_STATUS, &compileStatus);
		if (compileStatus != GL_TRUE) {
			const unsigned int buflen{ 1024 };
			GLchar log[buflen] = "";
			glGetShaderInfoLog(id, buflen, NULL, log);
			std::cerr << log << std::endl;

			return false;
		}

		return true;
	}

	// Check program linked without error (i.e. no errors in the shaders)
	bool LinkProgramShaders(GLuint shaderProgram)
	{
		// Do a link
		glLinkProgram(shaderProgram);

		GLint linkStatus = 0;
		glGetProgramiv(shaderProgram, GL_LINK_STATUS, &linkStatus);
		if (linkStatus != GL_TRUE) {
			const unsigned int buflen{ 1024 };
			GLchar log[buflen] = "";
			glGetShaderInfoLog(shaderProgram, buflen, NULL, log);
			std::cerr << log << std::endl;
			return false;
		}

		return true;
	}

	// Load and compile a shader of shaderType from file shaderFilename
	GLuint LoadAndCompileShader(GLenum shaderType, const std::string& shaderFilename)
	{
		// Create shaders
		GLuint shaderId{ glCreateShader(shaderType) };

		std::string vShaderString = stringFromFile(shaderFilename);
		if (vShaderString.empty())
		{
			std::cout << "Could not load " << shaderFilename << std::endl;
			return 0;
		}

		const char* asChar{ vShaderString.c_str() };

		std::cout << "Compiling Shader" << shaderFilename << std::endl;

		glShaderSource(shaderId, 1, (const GLchar * *)& asChar, NULL);
		glCompileShader(shaderId);

		if (!DidShaderCompileOK(shaderId))
			return 0;

		std::cout << "Compiled OK" << std::endl;

		return shaderId;
	}
}