#pragma once

#include "../IGameObject.h"

class TerrainCell;

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
			void Update() override;
			void SetProgram(GLuint aShaderName) override;
			void Destroy() override;

		private:
			std::vector<Vertex> vertices;
			std::vector<GLuint> indices;
			GLuint VAO, VBO, EBO;
			GLuint myProgram;
		};
	}
}