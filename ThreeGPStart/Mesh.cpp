#include "Mesh.h"
//#include <math.h>
//#define VERBOSE

#define M_PI 3.14159265358979323846264338327950288
#define EsAssert assert

namespace Helpers
{
	// Conversions from ASSIMP types
	inline glm::vec4 aiColor4DToGlmVec4(aiColor4D col) { return glm::vec4(col.r, col.g, col.b, col.a); }
	inline std::string aiStringToString(const aiString& str) { return std::string(str.C_Str()); }
	inline glm::vec3 aiVector3DToGlmVec3(aiVector3D vec) { return glm::vec3(vec.x, vec.y, vec.z); }

	// OpenGL uses column major matrices while ASSIMP uses row major - this converts
	inline glm::mat4 aiMatrix4x4ToGlm(const aiMatrix4x4* from)
	{
		glm::mat4 to;

		to[0][0] = (GLfloat)from->a1; to[0][1] = (GLfloat)from->b1;  to[0][2] = (GLfloat)from->c1; to[0][3] = (GLfloat)from->d1;
		to[1][0] = (GLfloat)from->a2; to[1][1] = (GLfloat)from->b2;  to[1][2] = (GLfloat)from->c2; to[1][3] = (GLfloat)from->d2;
		to[2][0] = (GLfloat)from->a3; to[2][1] = (GLfloat)from->b3;  to[2][2] = (GLfloat)from->c3; to[2][3] = (GLfloat)from->d3;
		to[3][0] = (GLfloat)from->a4; to[3][1] = (GLfloat)from->b4;  to[3][2] = (GLfloat)from->c4; to[3][3] = (GLfloat)from->d4;

		return to;
	}

	// x is roll, y is pitch, z is yaw
	glm::vec3 aiQuaternionToEulerAngles(aiQuaternion q) {
		glm::vec3 angles;

		// roll (x-axis rotation)
		double sinr_cosp = 2 * (double)(q.w * (double)q.x + q.y * (double)q.z);
		double cosr_cosp = 1 - 2 * (double)((double)q.x * q.x + (double)q.y * q.y);
		angles.x = (float)std::atan2(sinr_cosp, cosr_cosp);

		// pitch (y-axis rotation)
		double sinp = 2 * (double)((double)q.w * q.y - (double)q.z * q.x);
		if (std::abs(sinp) >= 1)
			angles.y = (float)std::copysign(M_PI / 2, sinp); // use 90 degrees if out of range
		else
			angles.y = (float)std::asin(sinp);

		// yaw (z-axis rotation)
		double siny_cosp = 2 * (double)((double)q.w * q.z + (double)q.x * q.y);
		double cosy_cosp = 1 - 2 * (double)((double)q.y * q.y + (double)q.z * q.z);
		angles.z = (float)std::atan2(siny_cosp, cosy_cosp);

		return angles;
	}

	// Retrieve the dimensions of this mesh in local coordinates
	void Mesh::GetLocalExtents(glm::vec3& minExtents, glm::vec3& maxExtents) const
	{
		if (vertices.empty())
			return;

		minExtents.x = maxExtents.x = vertices[0].x;
		minExtents.y = maxExtents.y = vertices[0].y;
		minExtents.z = maxExtents.z = vertices[0].z;

		for (size_t i = 1; i < vertices.size(); i++)
		{
			minExtents.x = std::min(minExtents.x, vertices[i].x);
			minExtents.y = std::min(minExtents.y, vertices[i].y);
			minExtents.z = std::min(minExtents.z, vertices[i].z);

			maxExtents.x = std::max(maxExtents.x, vertices[i].x);
			maxExtents.y = std::max(maxExtents.y, vertices[i].y);
			maxExtents.z = std::max(maxExtents.z, vertices[i].z);
		}
	}

	// Load a 3D model form a provided file and path, return false on error
	bool ModelLoader::LoadFromFile(const std::string& objFilename)
	{
		m_filename = objFilename;

#if defined(VERBOSE)
		std::cout << "\nUsing assimp to load: " << objFilename << std::endl;
#endif
		// Commom post processing steps - may slow load but make mesh better optimised
		unsigned int ppsteps = aiProcess_CalcTangentSpace | // calculate tangents and bitangents if possible
			aiProcess_JoinIdenticalVertices |				// join identical vertices/ optimize indexing
			aiProcess_ValidateDataStructure |				// perform a full validation of the loader's output
			aiProcess_ImproveCacheLocality |				// improve the cache locality of the output vertices
			aiProcess_RemoveRedundantMaterials |			// remove redundant materials
			aiProcess_FindDegenerates |						// remove degenerated polygons from the import
			aiProcess_FindInvalidData |						// detect invalid model data, such as invalid normal vectors
			aiProcess_GenUVCoords |							// convert spherical, cylindrical, box and planar mapping to proper UVs
			aiProcess_TransformUVCoords |					// preprocess UV transformations (scaling, translation ...)
			aiProcess_FindInstances |						// search for instanced meshes and remove them by references to one master
			aiProcess_LimitBoneWeights |					// limit bone weights to 4 per vertex
			aiProcess_OptimizeMeshes |						// join small meshes, if possible;
			aiProcess_SplitByBoneCount |					// split meshes with too many bones.
			aiProcess_GenSmoothNormals |					// generate smooth normal vectors if not existing
			aiProcess_SplitLargeMeshes |					// split large, unrenderable meshes into submeshes
			aiProcess_Triangulate |							// triangulate polygons with more than 3 edges
			aiProcess_SortByPType |							// make 'clean' meshes which consist of a single typ of primitives
			aiProcess_GenSmoothNormals |					// if no normals then create them
			aiProcess_GlobalScale |							// KD: Needed for FBX which uses cm rather than metres
			0;

		// Create an instance of the Importer class
		Assimp::Importer importer;

		// Buggy:
		// https://gamedev.stackexchange.com/questions/175044/assimp-skeletal-animation-with-some-fbx-files-has-issues-weird-node-added
		//importer.SetPropertyBool(AI_CONFIG_IMPORT_FBX_PRESERVE_PIVOTS, false);

		// By removing all points and lines we guarantee a face will describe a 3 vertex triangle
		importer.SetPropertyInteger(AI_CONFIG_PP_SBP_REMOVE, aiPrimitiveType_LINE | aiPrimitiveType_POINT);
		importer.SetPropertyInteger(AI_CONFIG_GLOB_MEASURE_TIME, 1);

		// KD: Need to scale down FBX which uses cm rather than metres
		if (objFilename.find(".fbx")!=std::string::npos)
			importer.SetPropertyFloat(AI_CONFIG_GLOBAL_SCALE_FACTOR_KEY, 0.01f);

		const aiScene* scene = importer.ReadFile(objFilename.c_str(), ppsteps);

		if (!scene)
		{
			std::cout << importer.GetErrorString() << std::endl;
			return false;
		}

		return PopulateFromAssimpScene(scene);
	}

	// Parse the ASSIMP data into our format
	bool ModelLoader::PopulateFromAssimpScene(const aiScene* scene)
	{
		// An assimp scene can contain many things I do not need like cameras and lights
		// Some I may want to support in the future so output that these exist but are being ignored:
#if defined(VERBOSE)
		if (scene->HasCameras())
			std::cout << "Ignoring: Scene has camera" << std::endl;
		if (scene->HasLights())
			std::cout << "Ignoring: Scene has lights" << std::endl;
#endif

		if (!scene->HasMeshes())
		{
			std::cerr << "Error: Scene has no mesh" << std::endl;
			return false;
		}

		// Materials are held scene wide and referenced in the part by id so need to grab here
		m_materials.resize(scene->mNumMaterials);
		for (unsigned int m = 0; m < scene->mNumMaterials; m++)
		{
			// Materials are held against property keys
			aiColor4D col;
			if (AI_SUCCESS == scene->mMaterials[m]->Get(AI_MATKEY_COLOR_AMBIENT, col))
				m_materials[m].ambientColour = aiColor4DToGlmVec4(col);

			if (AI_SUCCESS == scene->mMaterials[m]->Get(AI_MATKEY_COLOR_DIFFUSE, col))
				m_materials[m].diffuseColour = aiColor4DToGlmVec4(col);

			if (AI_SUCCESS == scene->mMaterials[m]->Get(AI_MATKEY_COLOR_SPECULAR, col))
				m_materials[m].specularColour = aiColor4DToGlmVec4(col);

			if (AI_SUCCESS == scene->mMaterials[m]->Get(AI_MATKEY_COLOR_EMISSIVE, col))
				m_materials[m].emissiveColour = aiColor4DToGlmVec4(col);

			unsigned int shininess = 0;
			unsigned int ret1 = scene->mMaterials[m]->Get(AI_MATKEY_SHININESS, shininess);
			unsigned int shininessStrength = 0;
			unsigned int ret2 = scene->mMaterials[m]->Get(AI_MATKEY_SHININESS_STRENGTH, shininessStrength);
			float specularFactor = 0;
			if (ret1 == AI_SUCCESS && ret2 == AI_SUCCESS)
				m_materials[m].specularFactor = (float)(shininess * shininessStrength);
			else if (ret1 == AI_SUCCESS)
				m_materials[m].specularFactor = (float)shininess; // TODO: not sure about this		

			// There are many types for each colour and also normals
			aiString texPath;
			if (AI_SUCCESS == scene->mMaterials[m]->GetTexture(aiTextureType_DIFFUSE, 0, &texPath))
			{
				// Just storing the filename for now, the factory will have to create a texture id
				m_materials[m].diffuseTextureFilename = std::string(texPath.C_Str());
			}

			if (AI_SUCCESS == scene->mMaterials[m]->GetTexture(aiTextureType_SPECULAR, 0, &texPath))
			{
				// Just storing the filename for now, the factory will have to create a texture id
				m_materials[m].specularTextureFilename = std::string(texPath.C_Str());
			}
#if defined(VERBOSE)
			// Ignoring others - report			 
			for (int i = aiTextureType_AMBIENT; i < aiTextureType_UNKNOWN; i++)
			{
				if (AI_SUCCESS == scene->mMaterials[m]->GetTexture((aiTextureType)i, 0, &texPath))
					std::cout << "Ignoring: material texture type: " + std::to_string(i) << std::endl;
			}
#endif
		}

		int hasBones{ 0 };
		int hasTangents{ 0 };
		int hasColourChannels{ 0 };
		int hasMMoreThanOneUVChannel{ 0 };

		//std::cout << "Scene contains " + std::to_string(scene->mNumMeshes) + " mesh");

		// ASSIMP mesh
		// http://assimp.sourceforge.net/lib_html/structai_mesh.html
		for (unsigned int i = 0; i < scene->mNumMeshes; i++)
		{
			aiMesh* aimesh = scene->mMeshes[i];

			if (aimesh->HasBones())
				hasBones++;
			if (aimesh->GetNumColorChannels())
				hasColourChannels++;
			if (aimesh->GetNumUVChannels() > 1)
				hasMMoreThanOneUVChannel++;
			if (aimesh->HasTangentsAndBitangents())
				hasTangents++;

			// Create my mesh part
			m_meshVector.push_back(Mesh());
			Mesh& newMesh = m_meshVector.back();

			newMesh.name = aimesh->mName.C_Str();

			//		std::cout << "Processing mesh with name: " + newMesh->name);

					// Copy over all the vertices, ai format of a vertex is same as mine
					// Probably be able to speed this all up at some point
			for (size_t v = 0; v < aimesh->mNumVertices; v++)
			{
				glm::vec3 newV = *(glm::vec3*) & aimesh->mVertices[v];
				newMesh.vertices.push_back(newV);
			}

			// And the normals if there are any
			if (aimesh->HasNormals())
			{
				for (size_t v = 0; v < aimesh->mNumVertices; v++)
				{
					glm::vec3 newN = *(glm::vec3*) & aimesh->mNormals[v];
					newMesh.normals.push_back(newN);
				}
			}

			// And texture coordinates
			if (aimesh->HasTextureCoords(0))
			{
				for (size_t v = 0; v < aimesh->mNumVertices; v++)
				{
					glm::vec2 newN = *(glm::vec2*) &aimesh->mTextureCoords[0][v];
					
					newMesh.uvCoords.push_back(newN);
				}
			}

			// Faces contain the vertex indices and due to the flags I set before are always triangles
			for (unsigned int face = 0; face < aimesh->mNumFaces; face++)
			{
				EsAssert(aimesh->mFaces[face].mNumIndices == 3);
				for (int triInd = 0; triInd < 3; triInd++)
					newMesh.elements.push_back(aimesh->mFaces[face].mIndices[triInd]);
			}

			// Material index
			newMesh.materialIndex = aimesh->mMaterialIndex;
		}
#if defined(VERBOSE)
		if (hasBones)
			std::cout << "Ignoring: One or more mesh have bones" << std::endl;
		if (hasColourChannels)
			std::cout << "Ignoring: One or more mesh has colour channels" << std::endl;
		if (hasMMoreThanOneUVChannel)
			std::cout << "Ignoring: One or more mesh has more than one UV channel" << std::endl;
		if (hasTangents)
			std::cout << "Ignoring: One or more mesh has tangents" << std::endl;
#endif
		// Hierarchy, ASSIMP calls these nodes
		m_rootNode = RecurseCreateNode(scene->mRootNode, nullptr);

		for (size_t i = 0; i < scene->mNumAnimations; i++)
		{
#if defined(VERBOSE)
			// Only supporting node animation			
			if (scene->mAnimations[i]->mNumMeshChannels)
				std::cout << "Ignoring: mesh animations" << std::endl;

			if (scene->mAnimations[i]->mNumChannels)
				std::cout << "Animation has " + std::to_string(scene->mAnimations[i]->mNumChannels) + " Channels" << std::endl;
#endif

			// Load the channel data
			for (unsigned int k = 0; k < scene->mAnimations[i]->mNumChannels; k++)
			{
				aiNodeAnim* node = scene->mAnimations[i]->mChannels[k];

#if defined(VERBOSE)
				std::cout << "Node: " + aiStringToString(node->mNodeName) << std::endl;
#endif

				Node* internalNode{ RecurseFindNode(m_rootNode,aiStringToString(node->mNodeName)) };
				if (!internalNode)
				{
					std::cout << "Failed to find internal node for channel animation" << std::endl;
					continue;
				}

#if defined(VERBOSE)
				std::cout << "Node has " + std::to_string(node->mNumPositionKeys) + " position keys" << std::endl;
				std::cout << "Node has " + std::to_string(node->mNumRotationKeys) + " rotation keys" << std::endl;
				std::cout << "Node has " + std::to_string(node->mNumScalingKeys) + " scaling keys" << std::endl;
#endif

				for (unsigned int j = 0; j < node->mNumPositionKeys; j++)
				{
					double time = node->mPositionKeys[j].mTime;
					aiVector3D val=node->mPositionKeys[j].mValue;

					internalNode->translationAnimationKeys.push_back(AnimationData{ (float)time, aiVector3DToGlmVec3(val) });
				}

				for (unsigned int j = 0; j < node->mNumRotationKeys; j++)
				{
					double time = node->mRotationKeys[j].mTime;
					aiQuaternion val = node->mRotationKeys[j].mValue;

					internalNode->translationAnimationKeys.push_back(AnimationData{ (float)time, aiQuaternionToEulerAngles(val) });					
				}

				for (unsigned int j = 0; j < node->mNumScalingKeys; j++)
				{
					double time = node->mScalingKeys[j].mTime;
					aiVector3D val = node->mScalingKeys[j].mValue;

					internalNode->translationAnimationKeys.push_back(AnimationData{ (float)time, aiVector3DToGlmVec3(val) });
				}				
			}
		}

		std::cout << "Loaded OK" << std::endl;

#if defined(VERBOSE)
		RecurseOutputHierarchy(m_rootNode, 0);
#endif

#if defined(VERBOSE)
		// Meta data
		if (scene->mMetaData)
		{
			for (unsigned int prop = 0; prop < scene->mMetaData->mNumProperties; prop++)
			{
				std::cout << "Meta data property " << prop << " key: " << aiStringToString(scene->mMetaData->mKeys[prop]) << std::endl;
				switch (scene->mMetaData->mValues[prop].mType)
				{
				case AI_BOOL:
					std::cout << "Value: " << *((bool*)scene->mMetaData->mValues[prop].mData) << std::endl;
					break;
				case AI_INT32:
					std::cout << "Value: " << *((int*)scene->mMetaData->mValues[prop].mData) << std::endl;
					break;
				case AI_UINT64:
					std::cout << "Value: " << *((unsigned int*)scene->mMetaData->mValues[prop].mData) << std::endl;
					break;
				case AI_FLOAT:
					std::cout << "Value: " << *((float*)scene->mMetaData->mValues[prop].mData) << std::endl;
					break;
				case AI_DOUBLE:
					std::cout << "Value: " << *((double*)scene->mMetaData->mValues[prop].mData) << std::endl;
					break;
				case AI_AISTRING:
					std::cout << "Value: " << *((std::string*)scene->mMetaData->mValues[prop].mData) << std::endl;
					break;
				case AI_AIVECTOR3D:
				{
					aiVector3D* vec = (aiVector3D*)scene->mMetaData->mValues[prop].mData;
					std::cout << "Value:" << vec->x << ", " << vec->y << ", " << vec->z << std::endl;
					break;
				}
				case AI_META_MAX: // ??
					std::cout << "Value:" << *((int*)scene->mMetaData->mValues[prop].mData) << std::endl;
					break;
				default:
					std::cout << "unknown meta data type" << std::endl;
					break;
				}
			}
		}
#endif
		return true;
	}

	void ModelLoader::RecurseOutputHierarchy(Node* node, int depth)
	{
		for (int i=0;i<depth;i++)
			std::cout << " ";

		glm::vec3 tran = glm::vec3(node->transform[3]);

		std::cout << "Node name: " << node->name << " Trans: " << tran.x << "," << tran.y << "," << tran.z << " Mesh: ";
		for (unsigned int m : node->meshIndices)
			std::cout << m << " ";
		std::cout << std::endl;

		for (auto n : node->childNodes)
			RecurseOutputHierarchy(n, depth + 1);
	}

	Node* ModelLoader::RecurseFindNode(Node* node, const std::string& nodeName)
	{
		if (!node)
			return nullptr;

		if (node->name == nodeName)
			return node;

		Node* retVal{ nullptr };
		for (auto child : node->childNodes)
		{
			retVal=RecurseFindNode(child, nodeName);
			if (retVal)
				break;
		}

		return retVal;
	}

	// Recursive node creation
	Node* ModelLoader::RecurseCreateNode(aiNode* node, Node* parent)
	{
		Node* newNode = new Node;

		newNode->name = node->mName.C_Str();
		newNode->parentNode = parent;

		for (size_t i = 0; i < node->mNumMeshes; i++)
			newNode->meshIndices.push_back(node->mMeshes[i]);

		newNode->transform = aiMatrix4x4ToGlm(&node->mTransformation);
		
		for (size_t i = 0; i < node->mNumChildren; i++)
			newNode->childNodes.emplace_back(RecurseCreateNode(node->mChildren[i], newNode));

		return newNode;
	}

	// Recursive deletion of nodes
	void ModelLoader::RecurseDeleteNode(Node* node)
	{
		if (!node)
			return;

		for (Node* n : node->childNodes)
			RecurseDeleteNode(n);

		delete node;
	}

	// Retrieve the dimensions of this model in local coordinates
	void ModelLoader::GetLocalExtents(glm::vec3& minExtents, glm::vec3& maxExtents) const
	{
		if (m_meshVector.empty())
			return;

		m_meshVector[0].GetLocalExtents(minExtents, maxExtents);

		for (size_t i = 1; i < m_meshVector.size(); i++)
		{
			glm::vec3 newMin;
			glm::vec3 newMax;
			m_meshVector[0].GetLocalExtents(newMin, newMax);

			minExtents.x = std::min(minExtents.x, newMin.x);
			minExtents.y = std::min(minExtents.y, newMin.y);
			minExtents.z = std::min(minExtents.z, newMin.z);

			maxExtents.x = std::max(maxExtents.x, newMax.x);
			maxExtents.y = std::max(maxExtents.y, newMax.y);
			maxExtents.z = std::max(maxExtents.z, newMax.z);
		}
	}
}