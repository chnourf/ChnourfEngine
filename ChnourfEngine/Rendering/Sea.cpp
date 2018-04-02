#include "Sea.h"
#include "../WorldGenerator/TerrainGenerationFunctions.h"
#include "glm\gtc\type_ptr.hpp"
#include "../Managers/ShaderManager.h"
#include "../Managers/SceneManager.h"

Sea::Sea()
{
	const auto halfTerrainSize = TerrainGeneration::GetMapSize()/2;
	GLfloat seaPlaneVertices[] =
	{
		-halfTerrainSize, 0.f, -halfTerrainSize,
		-halfTerrainSize, 0.f, halfTerrainSize,
		halfTerrainSize, 0.f, halfTerrainSize,

		-halfTerrainSize, 0.f, -halfTerrainSize,
		halfTerrainSize, 0.f, halfTerrainSize,
		halfTerrainSize, 0.f, -halfTerrainSize,
	};

	glGenVertexArrays(1, &myVao);
	glGenBuffers(1, &myVbo);
	glBindVertexArray(myVao);
	glBindBuffer(GL_ARRAY_BUFFER, myVbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(seaPlaneVertices), &seaPlaneVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
	glBindVertexArray(0);
}

void Sea::Render(GLuint aProgram)
{
	glUseProgram(aProgram);

	glBindVertexArray(myVao);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glDisable(GL_BLEND);
	glBindVertexArray(0);
}
