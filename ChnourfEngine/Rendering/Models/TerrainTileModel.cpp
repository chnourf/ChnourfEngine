#include "TerrainTileModel.h"
#include "../../WorldGenerator/TerrainTile.h"
#include "../../Managers/ModelManager.h"
#include "../Grass.h"
#include "../../Managers/SceneManager.h"
#include "../../Managers/ShaderManager.h"

#include "../../WorldGenerator/TerrainManager.h"

#include "../../Debug/WorldGridGeneratorDebug.h"


namespace Rendering
{
	namespace Models
	{
		std::vector<GLuint> TerrainTileModel::ourIndices[myNumLOD] = { std::vector<GLuint>(), std::vector<GLuint>(), std::vector<GLuint>(), std::vector<GLuint>() };

		void TerrainTileModel::CreateAndGenerateBuffers(GLuint& aVao, GLuint& aVbo, GLuint& aEbo, int aLod)
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

			// Normal
			glEnableVertexAttribArray(1);
			glVertexAttribIPointer(1, 1, GL_UNSIGNED_INT, sizeof(TerrainVertex), (GLvoid*)(GLvoid*)offsetof(TerrainVertex, normal));

			// Rainfall and temperature
			glEnableVertexAttribArray(2);
			glVertexAttribIPointer(2, 1, GL_UNSIGNED_INT, sizeof(TerrainVertex), (GLvoid*)(GLvoid*)offsetof(TerrainVertex, rainfallTemperatureErosion));

			glBindVertexArray(0);
			glBindBuffer(GL_ARRAY_BUFFER, 0);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		}

		void TerrainTileModel::AddFace(int aLod, unsigned int a, unsigned int b, unsigned int c)
		{
			ourIndices[aLod].push_back(a);
			ourIndices[aLod].push_back(b);
			ourIndices[aLod].push_back(c);
		}

		TerrainTileModel::TerrainTileModel(const TerrainTile* aTile, unsigned int aTileSize, float aResolution)
		{
			myTerrainTile = aTile;

			vec3f aMin = vec3f((float)myTerrainTile->GetGridIndex().x*aTileSize*aResolution, myTerrainTile->GetMinHeight(), (float)myTerrainTile->GetGridIndex().y*aTileSize*aResolution);
			vec3f aMax = vec3f((float) (myTerrainTile->GetGridIndex().x+1)*aTileSize*aResolution, myTerrainTile->GetMaxHeight(), (float) (myTerrainTile->GetGridIndex().y+1)*aTileSize*aResolution);

			myAABB = AABB(aMin, aMax);
			myPosition = vec3f((myTerrainTile->GetGridIndex().x + 0.5f)*aTileSize*aResolution, 0.f, (myTerrainTile->GetGridIndex().y + 0.5f)*aTileSize*aResolution);

			vertices.reserve(aTileSize*aTileSize);
			
			if (ourIndices[0].capacity() == 0)
			{
				for (unsigned int i = 0; i <= myMaxLOD; ++i)
				{
					//calculations to be remade
					ourIndices[i].reserve(7 * ((aTileSize - 1) / pow(2, i))*((aTileSize - 1) / pow(2, i)));
				}
			}
			
			for (unsigned short i = 0; i < aTileSize; ++i) {
				for (unsigned short j = 0; j < aTileSize; ++j) {
					auto& element = myTerrainTile->GetElement(i + j*aTileSize);

					unsigned char x = element.myNormal.x * 128 + 127;
					unsigned char y = element.myNormal.y * 128 + 127;
					unsigned char z = element.myNormal.z * 128 + 127;
					unsigned int normal = (x << 16) | (y << 8) | z;

					const unsigned char erosion = 255 * element.myErodedCoefficient;
					unsigned int rainfallAndTemperature = (element.myRainfall << 8) | element.myTemperature | (erosion << 16);

					TerrainVertex vertex = TerrainVertex(element.myElevation, normal, rainfallAndTemperature);
					vertices.push_back(vertex);
				}
			}

			// should be done for the first tile only
			if (ourIndices[0].size() == 0)
			{
				for (unsigned short i = 0; i < (aTileSize - 1); i++) {
					for (unsigned short j = 0; j < (aTileSize - 1); j++) {
						bool isEven = i % 2 == 0;
						//alternance even odd
						if (isEven)
						{
							AddFace(0,
								i + j * aTileSize,
								i + ((j + 1) % aTileSize)*aTileSize,
								(i + 1) % aTileSize + j*aTileSize);

							AddFace(0,
								(i + 1) % aTileSize + j*aTileSize,
								i + ((j + 1) % aTileSize)*aTileSize,
								(i + 1) % aTileSize + ((j + 1) % aTileSize)*aTileSize);
						}
						else
						{
							AddFace(0,
								i + j * aTileSize,
								(i + 1) % aTileSize + ((j + 1) % aTileSize)*aTileSize,
								(i + 1) % aTileSize + j*aTileSize);

							AddFace(0,
								i + j * aTileSize,
								i + ((j + 1) % aTileSize)*aTileSize,
								(i + 1) % aTileSize + ((j + 1) % aTileSize)*aTileSize);
						}
					}
				}


				for (unsigned int lod = 1; lod <= myMaxLOD; ++lod)
				{
					const int scale = pow(2, lod);
					for (unsigned short i = scale; i < (aTileSize - 2*scale); i += scale) {
						for (unsigned short j = scale; j < (aTileSize - 2 * scale); j += scale) {
							bool isEven = i % 2 == 0;
							//alternance even odd
							if (isEven)
							{
								AddFace(lod,
									i + j * aTileSize,
									i + ((j + scale) % aTileSize)*aTileSize,
									(i + scale) % aTileSize + j*aTileSize);

								AddFace(lod,
									(i + scale) % aTileSize + j*aTileSize,
									i + ((j + scale) % aTileSize)*aTileSize,
									(i + scale) % aTileSize + ((j + scale) % aTileSize)*aTileSize);
							}
							else
							{
								AddFace(lod,
									i + j * aTileSize,
									(i + scale) % aTileSize + ((j + scale) % aTileSize)*aTileSize,
									(i + scale) % aTileSize + j*aTileSize);

								AddFace(lod,
									i + j * aTileSize,
									i + ((j + scale) % aTileSize)*aTileSize,
									(i + scale) % aTileSize + ((j + scale) % aTileSize)*aTileSize);
							}
						}
					}

					//edges
					for (unsigned short i = 0; i < aTileSize; ++i) {
						for (unsigned short j = 0; j < aTileSize; ++j) {
							
							// top edge
							if (j == 0 && i < aTileSize - 1)
							{
								AddFace(lod,
									i,
									((i + scale / 2) / scale) * scale + aTileSize*scale,
									i + 1);

								if ((i + scale / 2) % scale == 0)
								{
									AddFace(lod,
										i,
										floor(i / scale) * scale + aTileSize * scale,
										floor(i / scale) * scale + (aTileSize + 1) * scale);
								}
							}

							// bottom edge
							if (j == aTileSize - 1 && i < aTileSize - 1)
							{
								AddFace(lod,
									i + aTileSize * j + 1,
									((i + scale / 2) / scale) * scale + aTileSize * (j - scale),
									i + aTileSize * j );

								if ((i + scale / 2) % scale == 0)
								{
									AddFace(lod,
										i + aTileSize * j,
										floor(i / scale) * scale + aTileSize * (j - scale) + scale,
										floor(i / scale) * scale + aTileSize * (j - scale));
								}
							}

							// left edge
							if (i == 0 && j > scale - 1 && j < aTileSize - scale - 1)
							{
								AddFace(lod,
									j * aTileSize,
									(j + 1) * aTileSize,
									((j + scale / 2) / scale) * scale * aTileSize + scale);

								if ((j + scale / 2) % scale == 0)
								{
									AddFace(lod,
										j * aTileSize,
										(j + scale / 2) / scale * scale * aTileSize + scale,
										(j + scale / 2) / scale * scale * aTileSize - aTileSize * scale + scale);
								}
							}

							// right edge
							if (i == aTileSize - 1 && j > scale - 1 && j < aTileSize - scale - 1)
							{
								AddFace(lod,
									i + j * aTileSize,
									((j + scale / 2) / scale) * scale * aTileSize + i - scale,
									i + (j + 1) * aTileSize);

								if ((j + scale / 2) % scale == 0)
								{
									AddFace(lod,
										i + j * aTileSize,
										(j + scale / 2) / scale * scale * aTileSize - aTileSize * scale + i - scale,
										(j + scale / 2) / scale * scale * aTileSize + i - scale);
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
			AddTexture("Data/TerrainGenerator/grassColor.png");
			AddTexture("Data/Grass/grass.png");

			myGrass = new Grass(aTileSize, aResolution, myTerrainTile->GetGridIndex().x);
		}

		void TerrainTileModel::AddTexture(const std::string& aString)
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
		
		TerrainTileModel::~TerrainTileModel()
		{
			for (int i = 0; i < myNumLOD; ++i)
			{
				glDeleteVertexArrays(1, &VAOs[i]);
				glDeleteBuffers(1, &VBOs[i]);
			}
			delete myGrass;
			Destroy();
		}

		void TerrainTileModel::Draw(const Manager::ShaderManager* aShaderManager)
		{
			myProgram = aShaderManager->GetShader("terrainShader");
			glUseProgram(myProgram);

			GLuint tileIndexID = glGetUniformLocation(myProgram, "tileIndex");
			auto& tileIndex = myTerrainTile->GetGridIndex();
			glUniform2i(tileIndexID, tileIndex.x, tileIndex.y);

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
			glUniform1i(glGetUniformLocation(myProgram, "grassColor"), 4);
			glActiveTexture(GL_TEXTURE4);
			glBindTexture(GL_TEXTURE_2D, textures[4].myId);

			glBindVertexArray(VAOs[myCurrentLOD]);
			glDrawElements(GL_TRIANGLES, (GLsizei)(ourIndices[myCurrentLOD].size()), GL_UNSIGNED_INT, 0);
			glBindVertexArray(0);

			if (myGrass->IsGenerated())
			{
				myGrass->Draw(aShaderManager, myTerrainTile->GetGridIndex(), textures[5].myId);
			}

			glBindTexture(GL_TEXTURE_2D, 0);
		}

		void TerrainTileModel::DrawForShadowMap(const Manager::ShaderManager* aShaderManager)
		{
			auto shadowProgramID = aShaderManager->GetShader("shadowMapTerrainShader");
			glUseProgram(shadowProgramID);
			GLuint tileIndexID = glGetUniformLocation(shadowProgramID, "tileIndex");
			auto& tileIndex = myTerrainTile->GetGridIndex();
			glUniform2i(tileIndexID, tileIndex.x, tileIndex.y);

			glBindVertexArray(VAOs[myCurrentLOD]);
			glDrawElements(GL_TRIANGLES, (GLsizei)(ourIndices[myCurrentLOD].size()), GL_UNSIGNED_INT, 0);
			glBindVertexArray(0);
		}

		void TerrainTileModel::Update()
		{
			const auto camPos = Manager::SceneManager::GetInstance()->GetCamPos();
			const auto tileSizeInMeters = myTerrainTile->GetTileSizeInMeters();
			const vec2i positionOnGrid = vec2i(camPos.x / tileSizeInMeters - 0.5f, camPos.z / tileSizeInMeters - 0.5f);
			auto& tileIndex = myTerrainTile->GetGridIndex();
			auto squareDist = (tileIndex.x - positionOnGrid.x) * (tileIndex.x - positionOnGrid.x) + (tileIndex.y - positionOnGrid.y) * (tileIndex.y - positionOnGrid.y);

			bool mustGenerateGrassIfNotDone = false;

			if (squareDist <= 9)
			{
				mustGenerateGrassIfNotDone = true;
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

			myGrass->Update(mustGenerateGrassIfNotDone, myTerrainTile);
		}

		void TerrainTileModel::SetProgram(GLuint aShaderName)
		{
			myProgram = aShaderName;
		}

		vec2i TerrainTileModel::GetGridIndex() const
		{
			return myTerrainTile->GetGridIndex();
		}

		void TerrainTileModel::CreateTexture(GLuint& aTextureID, const std::string& aPath)
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