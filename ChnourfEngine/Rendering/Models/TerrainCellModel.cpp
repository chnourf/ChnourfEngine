#include "TerrainCellModel.h"
#include "../../WorldGenerator/TerrainCell.h"
#include "../../Managers/ModelManager.h"
#include "../../Managers/SceneManager.h"
#include "../../Managers/ShaderManager.h"

namespace Rendering
{
	namespace Models
	{
		TerrainCellModel::TerrainCellModel(const TerrainCell* aCell, unsigned int aCellSize, float aResolution)
		{
			vertices.reserve(aCellSize*aCellSize);
			indices.reserve(6 * (aCellSize - 1)*(aCellSize - 1));

			// cell Resolution ?
			for (unsigned int i = 0; i < aCellSize; ++i) {     // y
				for (unsigned int j = 0; j < aCellSize; ++j) {  // x
					const float x = (float) (((float)j / ((float)(aCellSize-1)) + aCell->GetGridIndex().x) * aCellSize * aResolution);
					const float y = (float) (((float)i / ((float)(aCellSize-1)) + aCell->GetGridIndex().y) * aCellSize * aResolution);

					TerrainVertex vertex = TerrainVertex(glm::vec3(x, aCell->GetElement(i + j*aCellSize).myElevation-40, y), aCell->GetElement(i + j*aCellSize).myNormal);

					// we need to deduce the normals here

					vertices.push_back(vertex);
					bool isEven = i % 2 == 0;
					//alternance even odd
					if (i < aCellSize-1 && j < aCellSize-1)
					{
						if (isEven)
						{
							indices.push_back(i + j*aCellSize);
							indices.push_back(i + ((j + 1) % aCellSize)*aCellSize);
							indices.push_back((i + 1) % aCellSize + j*aCellSize);

							indices.push_back((i + 1) % aCellSize + j*aCellSize);
							indices.push_back(i + ((j + 1) % aCellSize)*aCellSize);
							indices.push_back((i + 1) % aCellSize + ((j + 1) % aCellSize)*aCellSize);
						}
						else
						{
							indices.push_back(i + j*aCellSize);
							indices.push_back((i+1) % aCellSize + ((j + 1) % aCellSize)*aCellSize);
							indices.push_back((i + 1) % aCellSize + j*aCellSize);

							indices.push_back(i + j*aCellSize);
							indices.push_back(i + ((j + 1) % aCellSize)*aCellSize);
							indices.push_back((i + 1) % aCellSize + ((j + 1) % aCellSize)*aCellSize);
						}
					}
				}
			}

			glGenVertexArrays(1, &VAO);
			glGenBuffers(1, &VBO);
			glGenBuffers(1, &EBO);

			glBindVertexArray(VAO);
			glBindBuffer(GL_ARRAY_BUFFER, VBO);

			glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(TerrainVertex),	&vertices[0], GL_STATIC_DRAW);

			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), &indices[0], GL_STATIC_DRAW);

			// Vertex Positions
			glEnableVertexAttribArray(0);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(TerrainVertex),	(GLvoid*)0);

			glEnableVertexAttribArray(1);
			glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(TerrainVertex), (GLvoid*)(GLvoid*)offsetof(TerrainVertex, normal));

			glBindVertexArray(0);
			glBindBuffer(GL_ARRAY_BUFFER, 0);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

			auto modelManager = Manager::SceneManager::GetInstance()->GetModelManager();
			const auto loadedTextures = modelManager->GetLoadedTextures();

			auto str = "Data/TerrainTest/terrain_d.jpg";

			auto skip = false;
			for (auto texture : loadedTextures)
			{
				if (std::strcmp(texture.myPath.C_Str(), str) == 0)
				{
					textures.push_back(texture);
					skip = true;
					break;
				}
			}

			if (!skip)
			{   // If texture hasn't been loaded already, load it
				TextureFormat texture;
				CreateTexture(texture.myId, str);
				texture.myPath = str;
				textures.push_back(texture);

				modelManager->AddLoadedTexture(texture);  // Add to loaded textures
			}

			auto str3 = "Data/TerrainTest/terrain_n.jpg";

			auto skip3 = false;
			for (auto texture : loadedTextures)
			{
				if (std::strcmp(texture.myPath.C_Str(), str3) == 0)
				{
					textures.push_back(texture);
					skip3 = true;
					break;
				}
			}

			if (!skip3)
			{   // If texture hasn't been loaded already, load it
				TextureFormat texture;
				CreateTexture(texture.myId, str3);
				texture.myPath = str3;
				textures.push_back(texture);

				modelManager->AddLoadedTexture(texture);  // Add to loaded textures
			}


			auto str2 = "Data/TerrainTest/rock_d.jpg";

			auto skip2 = false;
			for (auto texture : loadedTextures)
			{
				if (std::strcmp(texture.myPath.C_Str(), str2) == 0)
				{
					textures.push_back(texture);
					skip2 = true;
					break;
				}
			}

			if (!skip2)
			{   // If texture hasn't been loaded already, load it
				TextureFormat texture;
				CreateTexture(texture.myId, str2);
				texture.myPath = str2;
				textures.push_back(texture);

				modelManager->AddLoadedTexture(texture);  // Add to loaded textures
			}
		}


		TerrainCellModel::~TerrainCellModel()
		{
			glDeleteVertexArrays(1, &VAO);
			glDeleteBuffers(1, &VBO);
			Destroy();
		}

		void TerrainCellModel::Draw(const Manager::ShaderManager* aShaderManager)
		{
			myProgram = aShaderManager->GetShader("terrainShader");
			// needs to be done before all glUniform
			glUseProgram(myProgram);
			glBindVertexArray(VAO);
			glUniform1i(glGetUniformLocation(myProgram, "groundMaterial.diffuse"), 0);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, textures[0].myId);
			glUniform1i(glGetUniformLocation(myProgram, "rockMaterial.diffuse"),1);
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, textures[2].myId);
			glUniform1i(glGetUniformLocation(myProgram, "normalMap"), 2);
			glActiveTexture(GL_TEXTURE2);
			glBindTexture(GL_TEXTURE_2D, textures[1].myId);
			glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
			glBindVertexArray(0);
			glActiveTexture(GL_TEXTURE0);
		}

		void TerrainCellModel::DrawForShadowMap(const GLuint aShadowMapProgram)
		{
			// irrelevant for terrain but needed in shader. To optimise
			GLuint transformLoc1 = glGetUniformLocation(myProgram, "model");
			glUniformMatrix4fv(transformLoc1, 1, GL_FALSE, glm::value_ptr(glm::mat4(1.f)));

			glBindVertexArray(VAO);
			glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
			glBindVertexArray(0);
		}

		void TerrainCellModel::Update()
		{
			//this will be again overridden
		}

		void TerrainCellModel::SetProgram(GLuint aShaderName)
		{
			myProgram = aShaderName;
		}

		void TerrainCellModel::Destroy()
		{
		}

		void TerrainCellModel::CreateTexture(GLuint& aTextureID, const std::string& aPath)
		{
			glGenTextures(1, &aTextureID);
			glBindTexture(GL_TEXTURE_2D, aTextureID);
			int width, height;
			unsigned char* image = SOIL_load_image(aPath.c_str(), &width, &height, 0, SOIL_LOAD_RGBA);
			assert(image);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
			glGenerateMipmap(GL_TEXTURE_2D);
			SOIL_free_image_data(image);
			glBindTexture(GL_TEXTURE_2D, 0);
		}
	}
}