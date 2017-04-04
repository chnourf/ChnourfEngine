#include "Model.h"

#include "../../Core/Math.h"
#include "../../Dependencies/Assimp/Importer.hpp"
#include "../../Dependencies/Assimp/scene.h"
#include "../../Dependencies/Assimp/postprocess.h"
#include "../../Dependencies/SOIL/SOIL.h"
#include "../../Managers/ModelManager.h"
#include "../../Managers/SceneManager.h"

namespace Rendering
{
	namespace Models
	{
		Model::Model()
		{
		}

		Model::Model(const glm::mat4& aTransform)
		{
			myTransform = aTransform;
		}

		Model::Model(const glm::mat4& aTransform, const Material& aMaterial)
		{
			myTransform = aTransform;
			myMaterial = aMaterial;
		}

		Model::Model(const GLchar* aPath, const glm::mat4& aTransform)
		{
			myTransform = aTransform;

			myMaterial = Material(glm::vec3(.1f, .1f, .1f), glm::vec3(1.f, .5f, .31f), glm::vec3(.5f, .5f, .5f), 32.f);

			LoadModel(aPath);
		}

		void Model::LoadModel(const std::string& aPath)
		{
			Assimp::Importer importer;
			const aiScene* scene = importer.ReadFile(aPath, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenNormals);

			if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
			{
				std::cout << "ERROR::ASSIMP::" << importer.GetErrorString() << std::endl;
				return;
			}
			myDirectory = aPath.substr(0, aPath.find_last_of('/'));

			ProcessNode(scene->mRootNode, scene);
		}

		void Model::ProcessNode(aiNode* node, const aiScene* scene)
		{
			// Process all the node's meshes (if any)
			for (GLuint i = 0; i < node->mNumMeshes; i++)
			{
				aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
				myMeshes.push_back(ProcessMesh(mesh, scene));
			}
			// Then do the same for each of its children
			for (GLuint i = 0; i < node->mNumChildren; i++)
			{
				ProcessNode(node->mChildren[i], scene);
			}
		}

		Mesh* Model::ProcessMesh(aiMesh* mesh, const aiScene* scene)
		{
			std::vector<Vertex> vertices;
			std::vector<GLuint> indices;
			std::vector<TextureFormat> textures;

			for (GLuint i = 0; i < mesh->mNumVertices; i++)
			{
				Vertex vertex;

				glm::vec3 vector;
				vector.x = mesh->mVertices[i].x;
				vector.y = mesh->mVertices[i].y;
				vector.z = mesh->mVertices[i].z;
				vertex.position = vector;

				vector.x = mesh->mNormals[i].x;
				vector.y = mesh->mNormals[i].y;
				vector.z = mesh->mNormals[i].z;
				vertex.normal = vector;

				if (mesh->mTextureCoords[0]) // Does the mesh contain texture coordinates?
				{
					glm::vec2 vec;
					vec.x = mesh->mTextureCoords[0][i].x;
					vec.y = mesh->mTextureCoords[0][i].y;
					vertex.uv = vec;
				}
				else
				{
					vertex.uv = glm::vec2(0.0f, 0.0f);
				}

				vertices.push_back(vertex);
			}
			
			for (GLuint i = 0; i < mesh->mNumFaces; i++)
			{
				aiFace face = mesh->mFaces[i];
				for (GLuint j = 0; j < face.mNumIndices; j++)
					indices.push_back(face.mIndices[j]);
			}

				// Process material
			if (mesh->mMaterialIndex >= 0)
			{
				aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
				std::vector<TextureFormat> diffuseMaps = LoadMaterialTextures(material,	aiTextureType_DIFFUSE, TextureType::diffuse);
				textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
				std::vector<TextureFormat> specularMaps = LoadMaterialTextures(material, aiTextureType_SPECULAR, TextureType::normal);
				textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
			}

			return new Mesh(vertices, indices, textures);
		}

		std::vector<TextureFormat> Model::LoadMaterialTextures(aiMaterial* aMat, aiTextureType anAiType, TextureType aType)
		{
			std::vector<TextureFormat> textures;
			for (GLuint i = 0; i < aMat->GetTextureCount(anAiType); i++)
			{
				aiString str;
				aMat->GetTexture(anAiType, i, &str);
				TextureFormat texture;
				CreateTexture(texture.myId, myDirectory + '/' + str.data);

				auto modelManager = Manager::SceneManager::GetInstance()->GetModelManager();
				const auto loadedTextures = modelManager->GetLoadedTextures();

				auto skip = false;
				for (auto texture : loadedTextures)
				{
					if (std::strcmp(texture.myPath.C_Str(), str.C_Str()) == 0)
					{
						textures.push_back(texture);
						skip = true;
						break;
					}
				}

				if (!skip)
				{   // If texture hasn't been loaded already, load it
					TextureFormat texture;
					CreateTexture(texture.myId, myDirectory + '/' + str.C_Str());
					texture.myType = aType;
					texture.myPath = str;
					textures.push_back(texture);

					modelManager->AddLoadedTexture(texture);  // Add to loaded textures
				}
			}
			return textures;
		}

		Model::~Model()
		{
			Destroy();
		}

		void Model::Create()
		{
		}

		void Model::CreateTexture(GLuint& aTextureID, const std::string& aPath)
		{
			glGenTextures(1, &aTextureID);
			glBindTexture(GL_TEXTURE_2D, aTextureID);
			int width, height;
			unsigned char* image = SOIL_load_image(aPath.c_str(), &width, &height, 0, SOIL_LOAD_RGB);
			assert(image);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
			glGenerateMipmap(GL_TEXTURE_2D);
			SOIL_free_image_data(image);
			glBindTexture(GL_TEXTURE_2D, 0);
		}

		void Model::Draw(const Manager::ShaderManager* aShaderManager)
		{
			GLuint transformLoc1 = glGetUniformLocation(program, "model");
			glUniformMatrix4fv(transformLoc1, 1, GL_FALSE, glm::value_ptr(myTransform));


			glm::mat4 trans;
			trans = glm::rotate(trans, (float) -M_PI_2, glm::vec3(1.0, 0.0, 0.0));
			//trans = glm::rotate(trans, glutGet(GLUT_ELAPSED_TIME) * 1.0f / 10000.0f, glm::vec3(0.0, 0.0, 1.0));
			//trans = glm::translate(trans, glm::vec3(0.5f, -0.5f, 0.0f));

			GLuint transformLoc = glGetUniformLocation(program, "transform");
			glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(trans));

			GLint matAmbientLoc = glGetUniformLocation(program, "material.ambient");
			//GLint matDiffuseLoc = glGetUniformLocation(program, "material.diffuse");
			//GLint matSpecularLoc = glGetUniformLocation(program, "material.specular");
			GLint matShineLoc = glGetUniformLocation(program, "material.shininess");

			glUniform3f(matAmbientLoc, myMaterial.ambient.r, myMaterial.ambient.g, myMaterial.ambient.b);
			//glUniform3f(matDiffuseLoc, myMaterial.diffuse.r, myMaterial.diffuse.g, myMaterial.diffuse.b);
			//glUniform3f(matSpecularLoc, myMaterial.specular.r, myMaterial.specular.g, myMaterial.specular.b);
			glUniform1f(matShineLoc, myMaterial.shininess);
			glUseProgram(program);

			for (auto mesh : myMeshes)
			{
				mesh->Draw(program, aShaderManager);
			}
		}

		void Model::Update()
		{
			//this will be again overridden
		}

		void Model::SetProgram(GLuint aShaderName)
		{
			program = aShaderName;
		}

		void Model::Destroy()
		{
		}
	}
}