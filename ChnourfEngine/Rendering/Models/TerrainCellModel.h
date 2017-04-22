#pragma once

#include "../IGameObject.h"
#include "../TextureFormat.h"
#include "glm\detail\type_vec3.hpp"

class TerrainCell;

struct TerrainVertex
{
	glm::vec3 position;
	glm::vec3 normal;

	TerrainVertex()
	{
	}

	TerrainVertex(const glm::vec3& aPos, const glm::vec3& aNormal)
	{
		position = aPos;
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

			// right now this is very costly (for a 256x256 tile we store around 3.14MB !)
			std::vector<TerrainVertex> vertices;
			std::vector<GLuint> indices;
			std::vector<TextureFormat> textures;
			GLuint VAO, VBO, EBO;
			GLuint myProgram;
		};
	}
}