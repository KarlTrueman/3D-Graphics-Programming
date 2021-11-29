#pragma once

#include "ExternalLibraryHeaders.h"

/*
	Set of helpers for 3D Graphics Programming

	Changes 2021
	- switch to 4.6 profile
	- switched to use OpenGL debug callbacks
	- updated dependencies to latest
*/

namespace Helpers
{
	// Uses GLFW to set up a window via GLFW. Also initialises GLEW and OpenGL.
	GLFWwindow* CreateGLFWWindow(int width, int height, const std::string& title);

	// Loads a whole file into a string e.g. for shader use
	std::string stringFromFile(const std::string& filepath);

	// Check program linked without error (i.e. no errors in the shaders)
	bool LinkProgramShaders(GLuint shaderProgram);

	// Load and compile a shader of shaderType from file shaderFilename. Returns 0 on error.
	GLuint LoadAndCompileShader(GLenum shaderType, const std::string& shaderFilename);

	// Helper to output a glm::vec3
	inline std::string ToString(glm::vec3 v) {
		return "Pos x:" + std::to_string(v.x) +
			" y:" + std::to_string(v.y) +
			" z:" + std::to_string(v.z);
	};

	// Helper to output a glm::vec4 (as a colour)
	inline std::string ToString(glm::vec4 col)
	{
		return "R: " + std::to_string(col.r) +
			"G: " + std::to_string(col.g) +
			"B: " + std::to_string(col.b) +
			"A: " + std::to_string(col.a);
	}
};

