#pragma once

#include "../IGameObject.h"
#include "../TextureFormat.h"
#include "../../Core/Vector.h"
#include "glm\detail\type_vec3.hpp"

class TerrainTile;
class Grass;

struct TerrainVertex
{
	float elevation;
	unsigned int normal;
	unsigned int rainfallTemperatureErosion;

	TerrainVertex()
	{}

	TerrainVertex(const float anElevation, const unsigned int aNormal, const unsigned int aRainfallTemperatureErosion)
	{
		elevation = anElevation;
		normal = aNormal;
		rainfallTemperatureErosion = aRainfallTemperatureErosion;
	}
};

namespace Rendering
{
	namespace Models
	{
		class TerrainTileModel :public IGameObject
		{
		public:
			TerrainTileModel(const TerrainTile* aTile, unsigned int aTileSize, float aResolution);
			~TerrainTileModel();

			void Draw(const Manager::ShaderManager* aShaderManager) override;
			void DrawForShadowMap(const Manager::ShaderManager* aShaderManager) override;
			void Update() override;
			void SetProgram(GLuint aShaderName) override;

			vec2i GetGridIndex() const;

		private:
			void CreateTexture(GLuint& aTextureID, const std::string& aPath);
			void AddTexture(const std::string& aPath);
			void CreateAndGenerateBuffers(GLuint& aVao, GLuint& aVbo, GLuint& aEbo, int aLod);
			void AddFace(int aLod, unsigned int a, unsigned int b, unsigned int c);

			static const int myMaxLOD = 3;
			static const int myNumLOD = myMaxLOD + 1;
			static std::vector<GLuint> ourIndices[myNumLOD];

			std::vector<TerrainVertex> vertices;
			std::vector<TextureFormat> textures;
			GLuint VAOs[myNumLOD];
			GLuint VBOs[myNumLOD];
			GLuint EBOs[myNumLOD];
			GLuint myProgram;

			const TerrainTile* myTerrainTile;
			Grass* myGrass;

			unsigned int myCurrentLOD = 0;
		};
	}
}