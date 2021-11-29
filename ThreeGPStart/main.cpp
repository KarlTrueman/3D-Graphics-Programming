/* 
	main.cpp : This file contains the 'main' function. Program execution begins and ends there.

	A set of helper classes and start files for the 3D Graphics Programming module
	
	This project uses a number of helper libraries contained in the External folder within the project directory
	The ExternalLibraryHeaders.h loads their headers

	Visual Studio Project Setup
		The paths for the headers are set in project properties / C/C++ / General
		The paths for the libraries are set in project properties / Linker / General
		The static libraries (.lib) to link to are listed in Linker / Input
		There are also some runtime libraries (.dll) required. These are in the External/Bin directory. In order for Viz to find these
		during debugging the Debugging / environment is appended with the correct path
		If you run the exe outside of Viz these dlls need to be in the same folder as the exe but note that the provided
		MakeDistributable.bat batch file automatically copies them into the correct directory for you

	Libraries Used
	This project includes a number of helper libraries:
	
	GLFW - this is a small helper library used to handle the creation of a window, input etc. (https://www.glfw.org/) 
	GLEW - OpenGL is extended via extensions which need then to be mapped to fn calls. This takes away the pain (http://glew.sourceforge.net/)
	IMGUI - this is a simple GUI library for putting up windows, checkboxes, sliders etc. (https://github.com/ocornut/imgui)
	GLM - this is a maths library written to match with OpenGL calls. Supports vector and matrix math and quarternions etc. (https://github.com/g-truc/glm)

	These two are linked but you do not need to work with them directly as they are wrapped in helper code:

	FREEIMAGE - this is a library for loading image files. I wrap this in my own ImageLoader helper. (https://freeimage.sourceforge.io/)
	ASSIMP - this is a 3D model loading library. I wrap this in ModelLoader. (https://github.com/assimp/assimp)

	Important: of the provided files you should only need to edit the renderer.cpp and simulation.cpp files (plus of course add your own).

	Keith ditchburn 2021
*/

#include "ExternalLibraryHeaders.h"
#include "RedirectStandardOutput.h"

#include "Helper.h"
#include "Simulation.h"

// Note: you should not need to edit any of this
int main()
{	
	// Allows cout to go to the output pane in Visual Studio rather than have to open a console window
	RedirectStandardOuput();

	// Use the provided helper function to set up GLFW, GLEW and OpenGL
	GLFWwindow* window{ Helpers::CreateGLFWWindow(1280, 720, "3GP Framework") };
	if (!window)
		return -1;

	// Create an instance of the simulation class and initialise it
	// If it could not load, exit gracefully
	Simulation simulation;	
	if (!simulation.Initialise())
	{
		glfwTerminate();
		return -1;
	}
		
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GLFW_TRUE);

	// Enter main GLFW loop until the user closes the window
	while (!glfwWindowShouldClose(window))
	{				
		if (!simulation.Update(window))
			break;
		
		// GLFW updating
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	// Close down IMGUI
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	// Clean up and exit
	glfwDestroyWindow(window);
	glfwTerminate();

	return 0;
}