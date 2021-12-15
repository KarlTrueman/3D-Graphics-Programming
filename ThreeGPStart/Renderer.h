#pragma once

#include "ExternalLibraryHeaders.h"

#include "Helper.h"
#include "Mesh.h"
#include "Camera.h"

struct Mesh
{
	GLuint VAO;
	GLuint m_numElements;
	GLuint Tex;
};

struct Model
{
	std::vector<Mesh> m_meshVector;
};

class Renderer
{
private:
	Model Skymodel;
	std::vector<Model> m_modelVector;
	// Program object - to host shaders
	GLuint m_program{ 0 };
	//Skybox
	GLuint s_VAO{ 0 };
	GLuint s_tex;
	GLuint s_numElements{ 0 };
	//Terrain
	GLuint t_VAO{ 0 };
	GLuint t_tex;
	GLuint t_numElements{ 0 };
	// Vertex Array Object to wrap all render settings
	GLuint m_VAO{ 0 };
	GLuint tex{ 0 };
	// Number of elments to use when rendering
	GLuint m_numElements{ 0 };

	bool m_wireframe{ false };

	bool CreateProgram();

	bool Swap = false;
	bool NoiseGen = true;
	bool ExtraNoise;
	float NoiseVal{ 0 };
	size_t Index{ 0 };
public:
	Renderer();
	~Renderer();

	// Draw GUI
	void DefineGUI();

	// Create and / or load geometry, this is like 'level load'
	bool InitialiseGeometry();

	// Render the scene
	void Render(const Helpers::Camera& camera, float deltaTime);
};

