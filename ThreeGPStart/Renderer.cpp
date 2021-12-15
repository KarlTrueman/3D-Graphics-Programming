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
	return 1.0f - ((float)nn / 1073741924.0f);
}

// Load / create geometry into OpenGL buffers	
bool Renderer::InitialiseGeometry()
{

	// Load and compile shaders into m_program
	if (!CreateProgram())
		return false;


	// Load in the jeep
	Helpers::ModelLoader loader;
	if (!loader.LoadFromFile("Data\\Models\\Jeep\\jeep.obj"))
		return false;

	// Now we can loop through all the mesh in the loaded model:
	Helpers::ImageLoader texture;
	if (texture.Load("Data\\Models\\Jeep\\jeep_army.jpg"))
	{
		glGenTextures(1, &tex);
		glBindTexture(GL_TEXTURE_2D, tex);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texture.Width(), texture.Height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, texture.GetData());
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
	{
		MessageBox(NULL, L"Texture not found", L"Error", MB_OK | MB_ICONEXCLAMATION);
		return false;
	}

	for (const Helpers::Mesh& mesh : loader.GetMeshVector())
	{
		m_numElements = mesh.elements.size();

		GLuint meshVBO;
		glGenBuffers(1, &meshVBO);
		glBindBuffer(GL_ARRAY_BUFFER, meshVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * mesh.vertices.size(), mesh.vertices.data(), GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		GLuint normalsVBO;
		glGenBuffers(1, &normalsVBO);
		glBindBuffer(GL_ARRAY_BUFFER, normalsVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * mesh.normals.size(), mesh.normals.data(), GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		GLuint texcoordsVBO;
		glGenBuffers(1, &texcoordsVBO);
		glBindBuffer(GL_ARRAY_BUFFER, texcoordsVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec2) * mesh.uvCoords.size(), mesh.uvCoords.data(), GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		GLuint meshElementsEBO;
		glGenBuffers(1, &meshElementsEBO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, meshElementsEBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * mesh.elements.size(), mesh.elements.data(), GL_STATIC_DRAW);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

		std::vector<glm::vec3> verts;
		std::vector<GLuint> elements;
		std::vector<glm::vec3> colours;

		glGenVertexArrays(1, &m_VAO);
		glBindVertexArray(m_VAO);

		glBindBuffer(GL_ARRAY_BUFFER, meshVBO);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

		glBindBuffer(GL_ARRAY_BUFFER, normalsVBO);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

		glBindBuffer(GL_ARRAY_BUFFER, texcoordsVBO);
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, meshElementsEBO);
		glBindVertexArray(0);

	}
		//Terrain
		std::vector<glm::vec3 > tervertices;
		std::vector<GLuint> terelements;
		std::vector<glm::vec3 > ternormals;
		std::vector<glm::vec2 > tertexture;

		int numCellX = 1000;
		int numCellZ = 1000;

		int numVertX = 50;
		int numVertZ = 50;

		for (int i = 0; i < numVertX; i++)
		{
			for (int j = 0; j < numVertZ; j++)
			{
				tervertices.push_back(glm::vec3(i * 100, 0, j * 150));
				ternormals.push_back({ 0,1,0 });

				tertexture.push_back({ ((float)i / numVertZ) * 20, ((float)j / numVertX) *20 });
			}
		}

		for (int cellZ = 0; cellZ < numVertZ; cellZ++)
		{
			for (int cellX = 0; cellX < numVertX; cellX++)
			{
				int startVertIndex = (cellZ * numVertX) + cellX;
				if (Swap)
				{
					terelements.push_back(startVertIndex);
					terelements.push_back(startVertIndex + 1);
					terelements.push_back(startVertIndex + numVertX);

					terelements.push_back(startVertIndex + 1);
					terelements.push_back(startVertIndex + numVertX + 1);
					terelements.push_back(startVertIndex + numVertX);
				}
				else
				{
					terelements.push_back(startVertIndex);
					terelements.push_back(startVertIndex + numVertX + 1);
					terelements.push_back(startVertIndex + numVertX);

					terelements.push_back(startVertIndex + 1);
					terelements.push_back(startVertIndex + numVertX + 1);
					terelements.push_back(startVertIndex);
				}
				Swap = !Swap;
			}
			//Swap = !Swap;

		}
		if (NoiseGen)
		{
			for (int i = 0; i < numVertZ; i++)
			{
				for (int j = 0; j < numVertX; j++)
				{
					NoiseVal = Noise(i, j);
					NoiseVal = NoiseVal + 1.00001 / 2;
					glm::vec3 NoiseVec = tervertices[Index];
					
					NoiseVal = NoiseVal * 50;
					

					NoiseVec.y = NoiseVec.y + NoiseVal;
					tervertices[Index] = NoiseVec;
					Index++;
				}
			}
		}

		GLuint terpositionsVBO;
		glGenBuffers(1, &terpositionsVBO);
		glBindBuffer(GL_ARRAY_BUFFER, terpositionsVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * tervertices.size(), tervertices.data(), GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		GLuint terElementsEBO;
		glGenBuffers(1, &terElementsEBO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, terElementsEBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * terelements.size(), terelements.data(), GL_STATIC_DRAW);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

		GLuint ternormalsVBO;
		glGenBuffers(1, &ternormalsVBO);
		glBindBuffer(GL_ARRAY_BUFFER, ternormalsVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * ternormals.size(), ternormals.data(), GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		GLuint tertexcoordsVBO;
		glGenBuffers(1, &tertexcoordsVBO);
		glBindBuffer(GL_ARRAY_BUFFER, tertexcoordsVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec2) * tertexture.size(), tertexture.data(), GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		t_numElements = terelements.size();

		glGenVertexArrays(1, &t_VAO);
		glBindVertexArray(t_VAO);

		glBindBuffer(GL_ARRAY_BUFFER, terpositionsVBO);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,0,(void*)0);

		glBindBuffer(GL_ARRAY_BUFFER, ternormalsVBO);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

		glBindBuffer(GL_ARRAY_BUFFER, tertexcoordsVBO);
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, terElementsEBO);
		glBindVertexArray(0);


		if (texture.Load("Data\\Textures\\grass11.bmp"))
		{
			glGenTextures(1, &t_tex);
			glBindTexture(GL_TEXTURE_2D, t_tex);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texture.Width(), texture.Height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, texture.GetData());
			glGenerateMipmap(GL_TEXTURE_2D);
		}
		else
		{
			MessageBox(NULL, L"Texture not found", L"Error", MB_OK | MB_ICONEXCLAMATION);
			return false;
		}

		//Skybox
		Helpers::ModelLoader Skyloader;
		if (!Skyloader.LoadFromFile("Data\\Models\\Sky\\Mountains\\skybox.x"))
			return false;

		for (const Helpers::Mesh& mesh2 : Skyloader.GetMeshVector())
		{
			//m_numElements = mesh2.elements.size();
			Mesh newMesh;

			GLuint SkyMeshVBO;
			glGenBuffers(1, &SkyMeshVBO);
			glBindBuffer(GL_ARRAY_BUFFER, SkyMeshVBO);
			glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * mesh2.vertices.size(), mesh2.vertices.data(), GL_STATIC_DRAW);
			glBindBuffer(GL_ARRAY_BUFFER, 0);

			GLuint SkyNormalsVBO;
			glGenBuffers(1, &SkyNormalsVBO);
			glBindBuffer(GL_ARRAY_BUFFER, SkyNormalsVBO);
			glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * mesh2.normals.size(), mesh2.normals.data(), GL_STATIC_DRAW);
			glBindBuffer(GL_ARRAY_BUFFER, 0);

			GLuint SkyTexcoordsVBO;
			glGenBuffers(1, &SkyTexcoordsVBO);
			glBindBuffer(GL_ARRAY_BUFFER, SkyTexcoordsVBO);
			glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec2) * mesh2.uvCoords.size(), mesh2.uvCoords.data(), GL_STATIC_DRAW);
			glBindBuffer(GL_ARRAY_BUFFER, 0);

			GLuint SkyMeshElementsEBO;
			glGenBuffers(1, &SkyMeshElementsEBO);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, SkyMeshElementsEBO);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * mesh2.elements.size(), mesh2.elements.data(), GL_STATIC_DRAW);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

			newMesh.m_numElements = mesh2.elements.size();

			glGenVertexArrays(1, &newMesh.VAO);
			glBindVertexArray(newMesh.VAO);

			glBindBuffer(GL_ARRAY_BUFFER, SkyMeshVBO);
			glEnableVertexAttribArray(0);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

			glBindBuffer(GL_ARRAY_BUFFER, SkyNormalsVBO);
			glEnableVertexAttribArray(1);
			glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

			glBindBuffer(GL_ARRAY_BUFFER, SkyTexcoordsVBO);
			glEnableVertexAttribArray(2);
			glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, SkyMeshElementsEBO);
			glBindVertexArray(0);

			Skymodel.m_meshVector.emplace_back(newMesh);

		}
		m_modelVector.emplace_back(Skymodel);

		std::string facesCubemap[6] =
		{
			"Data\\Models\\Sky\\Mountains\\6.jpg",
			"Data\\Models\\Sky\\Mountains\\3.jpg",
			"Data\\Models\\Sky\\Mountains\\1.jpg",
			"Data\\Models\\Sky\\Mountains\\2.jpg",
			"Data\\Models\\Sky\\Mountains\\4.jpg",
			"Data\\Models\\Sky\\Mountains\\5.jpg"
		};

		for (int i = 0; i < Skymodel.m_meshVector.size(); i++)
		{
			Helpers::ImageLoader SkyBox_Texture;
			if (SkyBox_Texture.Load(facesCubemap[i]))
			{
				glGenTextures(1, &Skymodel.m_meshVector[i].Tex);
				glBindTexture(GL_TEXTURE_2D, Skymodel.m_meshVector[i].Tex);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, SkyBox_Texture.Width(), SkyBox_Texture.Height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, SkyBox_Texture.GetData());
				glGenerateMipmap(GL_TEXTURE_2D);
			}
		}
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
	glm::mat4 projection_xform = glm::perspective(glm::radians(45.0f), aspect_ratio, 1.0f, 40000.0f);

	// Compute camera view matrix and combine with projection matrix for passing to shader
	glm::mat4 view_xform = glm::lookAt(camera.GetPosition(), camera.GetPosition() + camera.GetLookVector(), camera.GetUpVector());


	// Use our program. Doing this enables the shaders we attached previously.
	glUseProgram(m_program);

	// Send the combined matrix to the shader in a uniform

	glm::mat4 model_xform = glm::mat4(1);

	// Uncomment all the lines below to rotate cube first round y then round x
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

	glDepthMask(GL_FALSE);
	glDisable(GL_DEPTH_TEST);

	//Skybox Render
	glm::mat4 view_xform2 = glm::mat4(glm::mat3(view_xform));
	glm::mat4 combined_xform2 = projection_xform * view_xform2;
	GLuint combined_xform_id2 = glGetUniformLocation(m_program, "combined_xform");
	glUniformMatrix4fv(combined_xform_id2, 1, GL_FALSE, glm::value_ptr(combined_xform2));
	for (int i = 0; i < Skymodel.m_meshVector.size(); i++)
	{
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, Skymodel.m_meshVector[i].Tex);
		glUniform1i(glGetUniformLocation(m_program, "sampelr_tex"), 0);
		glBindVertexArray(Skymodel.m_meshVector[i].VAO);
		glDrawElements(GL_TRIANGLES, Skymodel.m_meshVector[i].m_numElements, GL_UNSIGNED_INT, (void*)0);

	}
	glDepthMask(GL_TRUE);
	glEnable(GL_DEPTH_TEST);

	//Jeep render
	glm::mat4 combined_xform = projection_xform * view_xform;
	GLuint combined_xform_id = glGetUniformLocation(m_program, "combined_xform");
	glUniformMatrix4fv(combined_xform_id, 1, GL_FALSE, glm::value_ptr(combined_xform));
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, tex);
	glUniform1i(glGetUniformLocation(m_program, "sampelr_tex"), 0);

	model_xform = glm::translate(glm::mat4(1.0), glm::vec3{ 1000.0f, 0.0f, 500.0f });
	 
	// Send the model matrix to the shader in a uniform
	GLuint model_xform_id = glGetUniformLocation(m_program, "model_xform");
	glUniformMatrix4fv(model_xform_id, 1, GL_FALSE, glm::value_ptr(model_xform));

	// Bind our VAO and render
	glBindVertexArray(m_VAO);
	glDrawElements(GL_TRIANGLES, m_numElements, GL_UNSIGNED_INT, (void*)0);


	//Terrain Render
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, t_tex);

	model_xform = glm::mat4(1.0);
	glUniformMatrix4fv(model_xform_id, 1, GL_FALSE, glm::value_ptr(model_xform));
	
	//Bind vao and render
	glBindVertexArray(t_VAO);
	glDrawElements(GL_TRIANGLES, t_numElements, GL_UNSIGNED_INT, (void*)0);



}

