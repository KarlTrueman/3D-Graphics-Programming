#pragma once
// Mesh loading along with material handling and hierarchy nodes

#include "ExternalLibraryHeaders.h"
#include "Helper.h"

namespace Helpers
{
	// Per node animation data
	struct AnimationData
	{
		float time;
		glm::vec3 value;
	};

	// Materials work with lights and shaders to produce the final render
	struct Material
	{
		// Filename of diffuse texture (if any)
		std::string diffuseTextureFilename;

		// Filename of specular texture (if any)
		std::string specularTextureFilename;

		// Diffuse colour of material which will * by light diffuse based on distance and texel
		glm::vec4 diffuseColour{ 1 };

		// Ambient colour of material which will be * by global ambient and texel
		glm::vec4 ambientColour{ 1 };

		// Emissive colour of material which will be * by texel
		glm::vec4 emissiveColour{ 0 };

		// Specular colour * lighting colour and power and the specular factor modulated by angle and distance
		glm::vec4 specularColour{ 1 };

		// Specular factor, used as the power to modulate angle response by, shininess/glossiness Cannot be 0! Smaller = stronger
		float specularFactor{ 1.0f };

		// Helper to output material values for debugging purposes
		std::string ToString() const {
			return "Diffuse: " + Helpers::ToString(diffuseColour) +
				" Texture Filename: " + (diffuseTextureFilename.empty() ? " Noname " : diffuseTextureFilename) +
				" Ambient: " + Helpers::ToString(ambientColour) +
				" Emissive: " + Helpers::ToString(emissiveColour) +
				" Specular: " + Helpers::ToString(specularColour) +
				" Spec Factor: " + std::to_string(specularFactor) +
				" Texture: " + diffuseTextureFilename;
		}
	};

	// Data container for a mesh
	// A model can be made up of a number of mesh
	struct Mesh
	{
		// Name may well be blank, depends on the mesh creator
		std::string name;

		// Data in the mesh, vertices are guaranteed but normals and uvCoords depend on the model creator
		std::vector<glm::vec3> vertices;
		std::vector<glm::vec3> normals;
		std::vector<glm::vec2> uvCoords;

		// Elements
		std::vector<unsigned int> elements;

		// Index into the material vector held by the ModelLoader
		size_t materialIndex{ 0 };

		// Retrieve the dimensions of this mesh in local model coordinates
		void GetLocalExtents(glm::vec3& minExtents, glm::vec3& maxExtents) const;

		// Helper
		std::string ToString() const {
			return
				" Name: " + (name.empty() ? " Noname " : name) + "\n" +
				" Num Triangles: " + std::to_string(elements.size() / 3) + "\n" +
				" Num verts: " + std::to_string(vertices.size()) + "\n" +
				" Num normals: " + std::to_string(normals.size()) + "\n" +
				" Num uv coords: " + std::to_string(uvCoords.size()) + "\n" +
				" Num indices: " + std::to_string(elements.size());
		}
	};	

	// A mesh can contain a hierarchy in a tree structure
	// Each entry is a Node
	struct Node
	{
		std::string name;
		glm::mat4 transform{ 1 };
		std::vector<unsigned int> meshIndices;

		Node* parentNode{ nullptr };
		std::vector<Node*> childNodes;

		// Animations
		std::vector<AnimationData> translationAnimationKeys;
		std::vector<AnimationData> rotationAnimationKeys;
		std::vector<AnimationData> scaleAnimationKeys;
	};

	// Helper to load model data into mesh and material structures
	class ModelLoader
	{
	private:
		std::string m_filename;
		std::vector<Mesh> m_meshVector;
		std::vector<Material> m_materials;

		Node* m_rootNode{ nullptr };

		bool PopulateFromAssimpScene(const aiScene* scene);

		// Recursive
		Node* RecurseCreateNode(aiNode* node, Node* parent);
		void RecurseDeleteNode(Node* node);
		Node* RecurseFindNode(Node* node, const std::string& nodeName);
		void RecurseOutputHierarchy(Node* node, int depth);
	public:
		ModelLoader() = default;
		~ModelLoader() { RecurseDeleteNode(m_rootNode); }

		// Load a 3D model form a provided file and path, return false on error
		bool LoadFromFile(const std::string& objFilename);

		// Retrieves the collection of mesh loaded from the 3D model
		std::vector<Mesh>& GetMeshVector() { return m_meshVector; }

		// Retrieves the collection of materials loaded from the 3D model
		const std::vector<Material>& GetMaterialVector() const { return m_materials; }

		// For a mesh heirarchy this is the root.
		Node* GetRootNode() { return m_rootNode; }

		// Retrieve a specific node by name
		Node* FindNode(const std::string& nodeName) {
			return RecurseFindNode(m_rootNode, nodeName);
		}

		// Retrieve the dimensions of this model in local model coordinates
		void GetLocalExtents(glm::vec3& minExtents, glm::vec3& maxExtents) const;

		// Helper to output the main info. of this loaded model
		std::string ToString(bool describeEachMesh = true) const {
			std::string root = "File: " + m_filename + "\nNum mesh: " + std::to_string(m_meshVector.size()) +
				" Num materials: " + std::to_string(m_materials.size()) + "\n";
			if (describeEachMesh)
			{
				for (size_t i = 0; i < m_meshVector.size(); i++)
					root += "Mesh: " + std::to_string(i) + "\n" + m_meshVector[i].ToString();
			}
			return root;
		}
	};
}

