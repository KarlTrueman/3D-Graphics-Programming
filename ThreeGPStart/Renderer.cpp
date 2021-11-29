#include "Renderer.h"
#include "Camera.h"
#include "ImageLoader.h"

Renderer::Renderer()
{

}

// On exit must clean up any OpenGL resources e.g. the program, the buffers
Renderer::~Renderer()
{
	// TODO: clean up any memory used including OpenGL objects via glDelete* calls
	glDeleteProgram(m_program);
	glDeleteBuffers(1, &m_VAO);
}

// Use IMGUI for a simple on screen GUI
void Renderer::DefineGUI()
{
	// Show a simple window that we create ourselves. We use a Begin/End pair to created a named window.
	ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

	ImGui::Begin("3GP");						// Create a window called "3GP" and append into it.

	ImGui::Text("Visibility.");					// Display some text (you can use a format strings too)	

	ImGui::Checkbox("Wireframe", &m_wireframe);	// A checkbox linked to a member variable

	ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

	ImGui::End();
}

// Load, compile and link the shaders and create a program object to host them
bool Renderer::CreateProgram()
{
	// Create a new program (returns a unqiue id)
	m_program = glCreateProgram();

	// Load and create vertex and fragment shaders
	GLuint vertex_shader{ Helpers::LoadAndCompileShader(GL_VERTEX_SHADER, "Data/Shaders/vertex_shader.vert") };
	GLuint fragment_shader{ Helpers::LoadAndCompileShader(GL_FRAGMENT_SHADER, "Data/Shaders/fragment_shader.frag") };
	if (vertex_shader == 0 || fragment_shader == 0)
		return false;

	// Attach the vertex shader to this program (copies it)
	glAttachShader(m_program, vertex_shader);

	// The attibute location 0 maps to the input stream "vertex_position" in the vertex shader
	// Not needed if you use (location=0) in the vertex shader itself
	//glBindAttribLocation(m_program, 0, "vertex_position");

	// Attach the fragment shader (copies it)
	glAttachShader(m_program, fragment_shader);

	// Done with the originals of these as we have made copies
	glDeleteShader(vertex_shader);
	glDeleteShader(fragment_shader);

	// Link the shaders, checking for errors
	if (!Helpers::LinkProgramShaders(m_program))
		return false;

	return true;
}
float Noise(int x, int y)
{
	int n = x + y * 57;
	n = (n >> 13) ^ n;
	int nn = (n * (n * n * 60493 + 19990303) + 1376312589) & 0x7fffffff;
	return 1.0f - ((float)nn / 1073741824);
}

// Load / create geometry into OpenGL buffers	
bool Renderer::InitialiseGeometry()
{
	// Load and compile shaders into m_program
	if (!CreateProgram())
		return false;

	std::vector<glm::vec3 > vertices;
	std::vector<GLuint> elements;

	int numCellX = 20;
	int numCellZ = 20;

	int numVertX = numCellX + 1;
	int numVertZ = numCellZ + 1;

	for (int i = 0; i < numVertX; i++)
	{
		for (int j = 0; j < numVertZ; j++)
		{
			vertices.push_back(glm::vec3(i * 8, 0, j * 8));

		}
	}

	for (int cellZ = 0; cellZ < numCellZ; cellZ++)
	{
		for (int cellX = 0; cellX < numCellX; cellX++)
		{
			int startVertIndex = (cellZ * numVertX) + cellX;
			if (Swap)
			{
				elements.push_back(startVertIndex);
				elements.push_back(startVertIndex + 1);
				elements.push_back(startVertIndex + numVertX);

				elements.push_back(startVertIndex + 1);
				elements.push_back(startVertIndex + numVertX + 1);
				elements.push_back(startVertIndex + numVertX);
			}
			else
			{
				elements.push_back(startVertIndex);
				elements.push_back(startVertIndex + numVertX +1);
				elements.push_back(startVertIndex + numVertX);

				elements.push_back(startVertIndex + 1);
				elements.push_back(startVertIndex + numVertX + 1);
				elements.push_back(startVertIndex);
			}
			Swap = !Swap;
		}
		Swap = !Swap;
	}

	if (NoiseGen)
	{
		for (int i = 0; i < numVertX; i++)
		{
			for (int j = 0; j < numVertZ; j++)
			{
				NoiseVal = Noise(i, j);
				NoiseVal = NoiseVal + 1.25 / 2;
				glm::vec3 NoiseVec = vertices[Index];

				if (!ExtraNoise)
				{
					NoiseVal = NoiseVal * 5;
				}

				NoiseVec.y = NoiseVec.y + NoiseVal;
				vertices[Index] = NoiseVec;
				Index++;
			}
		}
	}

	GLuint positionsVBO;
	glGenBuffers(1, &positionsVBO);
	glBindBuffer(GL_ARRAY_BUFFER, positionsVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * vertices.size(), vertices.data(), GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	GLuint ElementsEBO;
	glGenBuffers(1, &ElementsEBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ElementsEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * elements.size(), elements.data(), GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	m_numElements = elements.size();

	glGenVertexArrays(1, &m_VAO);
	glBindVertexArray(m_VAO);
	glBindBuffer(GL_ARRAY_BUFFER, positionsVBO);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(
		0,
		3,
		GL_FLOAT,
		GL_FALSE,
		0,
		(void*)0
	);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ElementsEBO);
	glBindVertexArray(0);




	/*
		TODO 1: The first step is to create the vertices, colours and indices
		You could use std::vectors to hold these

		The cube is 20 by 20 centred on 0,0,0
		so the minimum values for x, y, z are -10,-10,-10
		and the maximum values are 10,10,10
	*/

	// Load in the jeep
	//Helpers::ModelLoader loader;
	//if (!loader.LoadFromFile("Data\\Models\\Jeep\\jeep.obj"))
	//	return false;

	//// Now we can loop through all the mesh in the loaded model:

	//Helpers::ImageLoader texture;
	//if (texture.Load("Data\\Models\\Jeep\\jeep_army.jpg"))
	//{
	//	glGenTextures(1, &tex);
	//	glBindTexture(GL_TEXTURE_2D, tex);
	//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	//	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texture.Width(), texture.Height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, texture.GetData());
	//	glGenerateMipmap(GL_TEXTURE_2D);
	//}
	//else
	//{
	//	MessageBox(NULL, L"Texture not found", L"Error", MB_OK | MB_ICONEXCLAMATION);
	//	return false;
	//}

	//for (const Helpers::Mesh& mesh : loader.GetMeshVector())
	//{
	//	m_numElements = mesh.elements.size();

	//	GLuint meshVBO;
	//	glGenBuffers(1, &meshVBO);
	//	glBindBuffer(GL_ARRAY_BUFFER, meshVBO);
	//	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * mesh.vertices.size(), mesh.vertices.data(), GL_STATIC_DRAW);
	//	glBindBuffer(GL_ARRAY_BUFFER, 0);

	//	//GLuint normalsVBO;
	//	//glGenBuffers(1, &normalsVBO);
	//	//glBindBuffer(GL_ARRAY_BUFFER, normalsVBO);
	//	//glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * mesh.normals.size(), mesh.normals.data(), GL_STATIC_DRAW);
	//	//glBindBuffer(GL_ARRAY_BUFFER, 0);

	//	GLuint meshElementsEBO;
	//	glGenBuffers(1, &meshElementsEBO);
	//	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, meshElementsEBO);
	//	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * mesh.elements.size(), mesh.elements.data(), GL_STATIC_DRAW);
	//	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	//	GLuint texcoordsVBO;
	//	glGenBuffers(1, &texcoordsVBO);
	//	glBindBuffer(GL_ARRAY_BUFFER, texcoordsVBO);
	//	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec2) * mesh.uvCoords.size(), mesh.uvCoords.data(), GL_STATIC_DRAW);
	//	glBindBuffer(GL_ARRAY_BUFFER, 0);

	//	std::vector<glm::vec3> verts;
	//	std::vector<GLuint> elements;
	//	std::vector<glm::vec3> colours;


	//	

	//	glGenVertexArrays(1, &m_VAO);
	//	glBindVertexArray(m_VAO);

	//	glBindBuffer(GL_ARRAY_BUFFER, meshVBO);
	//	glEnableVertexAttribArray(0);
	//	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

	//	//glBindBuffer(GL_ARRAY_BUFFER, normalsVBO);
	//	//glEnableVertexAttribArray(1);
	//	//glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

	//	glBindBuffer(GL_ARRAY_BUFFER, texcoordsVBO);
	//	glEnableVertexAttribArray(1);
	//	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

	//	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, meshElementsEBO);

	//	

	//}

	


	

	/*
		TODO 5: Run it and see if you can see the cube.
		You should not have to edit the render function or shaders to see the cube.
	*/

	return true;
}

// Render the scene. Passed the delta time since last called.
void Renderer::Render(const Helpers::Camera& camera, float deltaTime)
{
	// Configure pipeline settings
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	// Wireframe mode controlled by ImGui
	if (m_wireframe)
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	else
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	// Clear buffers from previous frame
	glClearColor(0.0f, 0.0f, 0.0f, 0.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Compute viewport and projection matrix
	GLint viewportSize[4];
	glGetIntegerv(GL_VIEWPORT, viewportSize);
	const float aspect_ratio = viewportSize[2] / (float)viewportSize[3];
	glm::mat4 projection_xform = glm::perspective(glm::radians(45.0f), aspect_ratio, 1.0f, 4000.0f);

	// Compute camera view matrix and combine with projection matrix for passing to shader
	glm::mat4 view_xform = glm::lookAt(camera.GetPosition(), camera.GetPosition() + camera.GetLookVector(), camera.GetUpVector());
	glm::mat4 combined_xform = projection_xform * view_xform;

	// Use our program. Doing this enables the shaders we attached previously.
	glUseProgram(m_program);

	// Send the combined matrix to the shader in a uniform
	GLuint combined_xform_id = glGetUniformLocation(m_program, "combined_xform");
	glUniformMatrix4fv(combined_xform_id, 1, GL_FALSE, glm::value_ptr(combined_xform));

	glm::mat4 model_xform = glm::mat4(1);

	//// Uncomment all the lines below to rotate cube first round y then round x
	//static float angle = 0;
	//static bool rotateY = true;

	//if (rotateY) // Rotate around y axis		
	//	model_xform = glm::rotate(model_xform, angle, glm::vec3{ 0 ,1,0 });
	//else // Rotate around x axis		
	//	model_xform = glm::rotate(model_xform, angle, glm::vec3{ 1 ,0,0 });

	//angle+=0.001f;
	//if (angle > glm::two_pi<float>())
	//{
	//	angle = 0;
	//	rotateY = !rotateY;
	//}

	// Send the model matrix to the shader in a uniform
	GLuint model_xform_id = glGetUniformLocation(m_program, "model_xform");
	glUniformMatrix4fv(model_xform_id, 1, GL_FALSE, glm::value_ptr(model_xform));

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, tex);
	glUniform1i(glGetUniformLocation(m_program, "sampelr_tex"), 0);


	// Bind our VAO and render
	glBindVertexArray(m_VAO);
	glDrawElements(GL_TRIANGLES, m_numElements, GL_UNSIGNED_INT, (void*)0);

}

