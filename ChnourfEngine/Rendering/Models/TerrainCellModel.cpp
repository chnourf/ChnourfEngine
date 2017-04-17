#include "TerrainCellModel.h"
#include "../../WorldGenerator/TerrainCell.h"
#include "../../Managers/ShaderManager.h"

namespace Rendering
{
	namespace Models
	{
		TerrainCellModel::TerrainCellModel(const TerrainCell* aCell, unsigned int aCellSize, float aResolution)
		{
			// cell Resolution ?
			for (unsigned int i = 0; i < aCellSize; ++i) {     // y
				for (unsigned int j = 0; j < aCellSize; ++j) {  // x
					const float x = (float) (((float)j / ((float)aCellSize) + aCell->GetGridIndex().x) * aCellSize * aResolution);
					const float y = (float) (((float)i / ((float)aCellSize) + aCell->GetGridIndex().y) * aCellSize * aResolution);

					Vertex vertex = Vertex(glm::vec3(x, aCell->GetElement(i + j*aCellSize).myElevation/3000.f, y), glm::vec3(0, 1, 0), glm::vec2(0,0));
					vertices.push_back(vertex);

					indices.push_back(i + j*aCellSize);
					indices.push_back(i + ((j+1)%aCellSize)*aCellSize);
					indices.push_back((i + 1)%aCellSize  + j*aCellSize);

					indices.push_back((i + 1) % aCellSize + j*aCellSize);
					indices.push_back(i + ((j + 1) % aCellSize)*aCellSize);
					indices.push_back((i + 1) % aCellSize + ((j + 1) % aCellSize)*aCellSize);
				}
			}

			glGenVertexArrays(1, &VAO);
			glGenBuffers(1, &VBO);
			glGenBuffers(1, &EBO);

			glBindVertexArray(VAO);
			glBindBuffer(GL_ARRAY_BUFFER, VBO);

			glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex),
				&vertices[0], GL_STATIC_DRAW);

			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint),
				&indices[0], GL_STATIC_DRAW);

			// Vertex Positions
			glEnableVertexAttribArray(0);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
				(GLvoid*)0);

			glBindVertexArray(0);
			glBindBuffer(GL_ARRAY_BUFFER, 0);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		}


		TerrainCellModel::~TerrainCellModel()
		{
			glDeleteVertexArrays(1, &VAO);
			glDeleteBuffers(1, &VBO);
			Destroy();
		}

		void TerrainCellModel::Draw(const Manager::ShaderManager* aShaderManager)
		{
			// needs to be done before all glUniform
			glUseProgram(aShaderManager->GetShader("terrainShader"));
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
	}
}