#include "TerrainCellModel.h"
#include "../../WorldGenerator/TerrainCell.h"
#include "../../Managers/ModelManager.h"
#include "../../Managers/SceneManager.h"
#include "../../Managers/ShaderManager.h"

namespace Rendering
{
	namespace Models
	{
		std::vector<GLuint> TerrainCellModel::ourIndices[myNumLOD] = { std::vector<GLuint>(), std::vector<GLuint>(), std::vector<GLuint>(), std::vector<GLuint>() };

		void TerrainCellModel::CreateAndGenerateBuffers(GLuint& aVao, GLuint& aVbo, GLuint& aEbo, int aLod)
		{
			glGenVertexArrays(1, &aVao);
			glGenBuffers(1, &aVbo);
			glGenBuffers(1, &aEbo);

			glBindVertexArray(aVao);
			glBindBuffer(GL_ARRAY_BUFFER, aVbo);
			glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(TerrainVertex), &vertices[0], GL_STATIC_DRAW);

			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, aEbo);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, ourIndices[aLod].size() * sizeof(GLuint), &ourIndices[aLod][0], GL_STATIC_DRAW);

			// Vertex Positions
			glEnableVertexAttribArray(0);
			glVertexAttribPointer(0, 1, GL_FLOAT, GL_FALSE, sizeof(TerrainVertex), (GLvoid*)0);

			glEnableVertexAttribArray(1);
			glVertexAttribIPointer(1, 1, GL_UNSIGNED_INT, sizeof(TerrainVertex), (GLvoid*)(GLvoid*)offsetof(TerrainVertex, normal));

			glBindVertexArray(0);
			glBindBuffer(GL_ARRAY_BUFFER, 0);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		}

		void TerrainCellModel::AddFace(int aLod, unsigned int a, unsigned int b, unsigned int c)
		{
			ourIndices[aLod].push_back(a);
			ourIndices[aLod].push_back(b);
			ourIndices[aLod].push_back(c);
		}

		TerrainCellModel::TerrainCellModel(const TerrainCell* aCell, unsigned int aCellSize, float aResolution)
		{
			vec3f aMin = vec3f((float) aCell->GetGridIndex().x*aCellSize*aResolution, aCell->GetMinHeight(), (float) aCell->GetGridIndex().y*aCellSize*aResolution);
			vec3f aMax = vec3f((float) (aCell->GetGridIndex().x+1)*aCellSize*aResolution, aCell->GetMaxHeight(), (float) (aCell->GetGridIndex().y+1)*aCellSize*aResolution);

			myAABB = AABB(aMin, aMax);

			myPosition = vec3f((aCell->GetGridIndex().x + 0.5)*aCellSize*aResolution, 0.f, (aCell->GetGridIndex().y + 0.5)*aCellSize*aResolution);

			vertices.reserve(aCellSize*aCellSize);
			
			if (ourIndices[0].capacity() == 0)
			{
				for (unsigned int i = 0; i <= myMaxLOD; ++i)
				{
					//calculations to be remade
					ourIndices[i].reserve(7 * ((aCellSize - 1) / pow(2, i))*((aCellSize - 1) / pow(2, i)));
					//ourIndices[i].reserve(6 * ((aCellSize-1) / pow(2, i))*((aCellSize - 1) / pow(2, i)));
				}
			}
			
			myTileIndex = aCell->GetGridIndex();

			for (unsigned short i = 0; i < aCellSize; ++i) {
				for (unsigned short j = 0; j < aCellSize; ++j) {
					auto& element = aCell->GetElement(i + j*aCellSize);

					unsigned char x = element.myNormal.x * 128 + 128;
					unsigned char y = element.myNormal.y * 128 + 128;
					unsigned char z = element.myNormal.z * 128 + 128;
					unsigned int normal = (x << 16) | (y << 8) | z;

					TerrainVertex vertex = TerrainVertex(element.myElevation, normal);
					vertices.push_back(vertex);
				}
			}

			// should be done for the first cell only
			if (ourIndices[0].size() == 0)
			{
				for (unsigned short i = 0; i < (aCellSize - 1); i++) {
					for (unsigned short j = 0; j < (aCellSize - 1); j++) {
						bool isEven = i % 2 == 0;
						//alternance even odd
						if (isEven)
						{
							AddFace(0,
								i + j * aCellSize,
								i + ((j + 1) % aCellSize)*aCellSize,
								(i + 1) % aCellSize + j*aCellSize);

							AddFace(0,
								(i + 1) % aCellSize + j*aCellSize,
								i + ((j + 1) % aCellSize)*aCellSize,
								(i + 1) % aCellSize + ((j + 1) % aCellSize)*aCellSize);
						}
						else
						{
							AddFace(0,
								i + j * aCellSize,
								(i + 1) % aCellSize + ((j + 1) % aCellSize)*aCellSize,
								(i + 1) % aCellSize + j*aCellSize);

							AddFace(0,
								i + j * aCellSize,
								i + ((j + 1) % aCellSize)*aCellSize,
								(i + 1) % aCellSize + ((j + 1) % aCellSize)*aCellSize);
						}
					}
				}


				for (unsigned int lod = 1; lod <= myMaxLOD; ++lod)
				{
					const int scale = pow(2, lod);
					for (unsigned short i = scale; i < (aCellSize - 2*scale); i += scale) {
						for (unsigned short j = scale; j < (aCellSize - 2 * scale); j += scale) {
							bool isEven = i % 2 == 0;
							//alternance even odd
							if (isEven)
							{
								AddFace(lod,
									i + j * aCellSize,
									i + ((j + scale) % aCellSize)*aCellSize,
									(i + scale) % aCellSize + j*aCellSize);

								AddFace(lod,
									(i + scale) % aCellSize + j*aCellSize,
									i + ((j + scale) % aCellSize)*aCellSize,
									(i + scale) % aCellSize + ((j + scale) % aCellSize)*aCellSize);
							}
							else
							{
								AddFace(lod,
									i + j * aCellSize,
									(i + scale) % aCellSize + ((j + scale) % aCellSize)*aCellSize,
									(i + scale) % aCellSize + j*aCellSize);

								AddFace(lod,
									i + j * aCellSize,
									i + ((j + scale) % aCellSize)*aCellSize,
									(i + scale) % aCellSize + ((j + scale) % aCellSize)*aCellSize);
							}
						}
					}

					//edges
					for (unsigned short i = 0; i < aCellSize; ++i) {
						for (unsigned short j = 0; j < aCellSize; ++j) {
							
							// top edge
							if (j == 0 && i < aCellSize - 1)
							{
								AddFace(lod,
									i,
									((i + scale / 2) / scale) * scale + aCellSize*scale,
									i + 1);

								if ((i + scale / 2) % scale == 0)
								{
									AddFace(lod,
										i,
										floor(i / scale) * scale + aCellSize * scale,
										floor(i / scale) * scale + (aCellSize + 1) * scale);
								}
							}

							// bottom edge
							if (j == aCellSize - 1 && i < aCellSize - 1)
							{
								AddFace(lod,
									i + aCellSize * j + 1,
									((i + scale / 2) / scale) * scale + aCellSize * (j - scale),
									i + aCellSize * j );

								if ((i + scale / 2) % scale == 0)
								{
									AddFace(lod,
										i + aCellSize * j,
										floor(i / scale) * scale + aCellSize * (j - scale) + scale,
										floor(i / scale) * scale + aCellSize * (j - scale));
								}
							}

							// left edge
							if (i == 0 && j > scale - 1 && j < aCellSize - scale - 1)
							{
								AddFace(lod,
									j * aCellSize,
									(j + 1) * aCellSize,
									((j + scale / 2) / scale) * scale * aCellSize + scale);

								if ((j + scale / 2) % scale == 0)
								{
									AddFace(lod,
										j * aCellSize,
										(j + scale / 2) / scale * scale * aCellSize + scale,
										(j + scale / 2) / scale * scale * aCellSize - aCellSize * scale + scale);
								}
							}

							// right edge
							if (i == aCellSize - 1 && j > scale - 1 && j < aCellSize - scale - 1)
							{
								AddFace(lod,
									i + j * aCellSize,
									((j + scale / 2) / scale) * scale * aCellSize + i - scale,
									i + (j + 1) * aCellSize);

								if ((j + scale / 2) % scale == 0)
								{
									AddFace(lod,
										i + j * aCellSize,
										(j + scale / 2) / scale * scale * aCellSize - aCellSize * scale + i - scale,
										(j + scale / 2) / scale * scale * aCellSize + i - scale);
								}
							}
						}
					}
				}
			}

			for (unsigned int lod = 0; lod <= myMaxLOD; ++lod)
			{
				CreateAndGenerateBuffers(VAOs[lod], VBOs[lod], EBOs[lod], lod);
			}

			AddTexture("Data/TerrainTest/terrain_d.jpg");
			AddTexture("Data/TerrainTest/terrain_n.jpg");
			AddTexture("Data/TerrainTest/rock_d.jpg");
			AddTexture("Data/TerrainTest/snow_d.jpg");

			auto& grassPos = aCell->GetGrassSpots();
			myGrassPositions.reserve(grassPos.size());
			for (auto grass : grassPos)
			{
				myGrassPositions.push_back(glm::vec3(grass.x, grass.y, grass.z));
			}
		}

		void TerrainCellModel::AddTexture(const std::string& aString)
		{
			auto modelManager = Manager::SceneManager::GetInstance()->GetModelManager();
			const auto loadedTextures = modelManager->GetLoadedTextures();

			auto skip = false;
			for (auto texture : loadedTextures)
			{
				if (std::strcmp(texture.myPath.C_Str(), aString.c_str()) == 0)
				{
					textures.push_back(texture);
					skip = true;
					break;
				}
			}

			if (!skip)
			{   // If texture hasn't been loaded already, load it
				TextureFormat texture;
				CreateTexture(texture.myId, aString);
				texture.myPath = aString;
				textures.push_back(texture);

				modelManager->AddLoadedTexture(texture);  // Add to loaded textures
			}
		}


		TerrainCellModel::~TerrainCellModel()
		{
			glDeleteVertexArrays(1, &VAOs[0]);
			glDeleteBuffers(1, &VBOs[0]);
			Destroy();
		}

		void TerrainCellModel::Draw(const Manager::ShaderManager* aShaderManager)
		{
			myProgram = aShaderManager->GetShader("terrainShader");
			glUseProgram(myProgram);

			GLuint powCurrentLOD = glGetUniformLocation(myProgram, "powCurrentLOD");
			glUniform1i(powCurrentLOD, 1);// pow(2, myCurrentLOD));

			GLuint tileIndex = glGetUniformLocation(myProgram, "tileIndex");
			glUniform2i(tileIndex, myTileIndex.x, myTileIndex.y);
			glUniform1i(glGetUniformLocation(myProgram, "groundMaterial.diffuse"), 0);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, textures[0].myId);
			glUniform1i(glGetUniformLocation(myProgram, "rockMaterial.diffuse"),1);
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, textures[2].myId);
			glUniform1i(glGetUniformLocation(myProgram, "normalMap"), 2);
			glActiveTexture(GL_TEXTURE2);
			glBindTexture(GL_TEXTURE_2D, textures[1].myId);
			glUniform1i(glGetUniformLocation(myProgram, "snowMaterial.diffuse"), 3);
			glActiveTexture(GL_TEXTURE3);
			glBindTexture(GL_TEXTURE_2D, textures[3].myId);

			glBindVertexArray(VAOs[myCurrentLOD]);
			glDrawElements(GL_TRIANGLES, (GLsizei)(ourIndices[myCurrentLOD].size()), GL_UNSIGNED_INT, 0);
			glBindVertexArray(0);
		}

		void TerrainCellModel::DrawForShadowMap(const GLuint aShadowMapProgram)
		{
			// irrelevant for terrain but needed in shader. To optimise
			GLuint transformLoc1 = glGetUniformLocation(myProgram, "model");
			glUniformMatrix4fv(transformLoc1, 1, GL_FALSE, glm::value_ptr(glm::mat4(1.f)));

			glBindVertexArray(VAOs[myCurrentLOD]);
			glDrawElements(GL_TRIANGLES, (GLsizei)(ourIndices[myCurrentLOD].size()), GL_UNSIGNED_INT, 0);
			glBindVertexArray(0);
		}

		void TerrainCellModel::Update()
		{
			auto camPos = Manager::SceneManager::GetInstance()->GetCamPos();
			const vec2i positionOnGrid = vec2i(camPos.x / (128), camPos.z / (128));
			// ugly
			auto squareDist = (myTileIndex.x - positionOnGrid.x) * (myTileIndex.x - positionOnGrid.x) + (myTileIndex.y - positionOnGrid.y) * (myTileIndex.y - positionOnGrid.y);
			if (squareDist <= 9)
			{
				myCurrentLOD = 0;
			}
			else if (squareDist <= 16)
			{
				myCurrentLOD = 1;
			}
			else if (squareDist <= 25)
			{
				myCurrentLOD = 2;
			}
			else
			{
				myCurrentLOD = 3;
			}
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