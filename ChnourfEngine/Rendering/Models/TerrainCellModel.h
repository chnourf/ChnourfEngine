#pragma once

#include "../IGameObject.h"
#include "../TextureFormat.h"
#include "../../Core/Vector.h"
#include "glm\detail\type_vec3.hpp"

class TerrainCell;

struct TerrainVertex
{
	float elevation;
	unsigned int normal;

	TerrainVertex()
	{}

	TerrainVertex(const float anElevation, const unsigned int aNormal)
	{
		elevation = anElevation;
		normal = aNormal;
	}
};

namespace Rendering
{
	namespace Models
	{
		class TerrainCellModel :public IGameObject
		{
		public:
			TerrainCellModel(const TerrainCell* aCell, unsigned int aCellSize, float aResolution);
			~TerrainCellModel();

			void Draw(const Manager::ShaderManager* aShaderManager) override;
			void DrawForShadowMap(const GLuint aShadowMapProgram) override;
			void Update() override;
			void SetProgram(GLuint aShaderName) override;
			void Destroy() override;

		private:
			void CreateTexture(GLuint& aTextureID, const std::string& aPath);
			void AddTexture(const std::string& aPath);
			void DrawGrass(const Manager::ShaderManager* aShaderManager);
			void CreateAndGenerateBuffers(GLuint& aVao, GLuint& aVbo, GLuint& aEbo, int aLod);
			void AddFace(int aLod, unsigned int a, unsigned int b, unsigned int c);

			static const int myMaxLOD = 3;
			static const int myNumLOD = myMaxLOD + 1;
			static std::vector<GLuint> ourIndices[myNumLOD];

			std::vector<glm::vec3> myGrassPositions;
			GLuint myGrassVAO;
			GLuint myGrassVBO;
			GLuint myGrassProgram;

			std::vector<TerrainVertex> vertices;
			std::vector<TextureFormat> textures;
			GLuint VAOs[myNumLOD];
			GLuint VBOs[myNumLOD];
			GLuint EBOs[myNumLOD];
			GLuint myProgram;

			vec2i myTileIndex;

			unsigned int myCurrentLOD = 0;
		};
	}
}