#include "Grass.h"

#include "../WorldGenerator/TerrainCell.h"
#include "../Managers/ShaderManager.h"

Grass::Grass(unsigned int aCellSize, float aResolution, int aSeed):
	myCellSize(aCellSize),
	myResolution(aResolution),
	mySeed(aSeed)
{
	myDensityPerSqMeter = 4;
}

void Grass::GenerateGrass(const TerrainCell* aCell)
{
	if (myGrassData.size() > 0)
	{
		return;
	}

	// resetting engine
	myRandomEngine.seed(mySeed);
	const float offset = 0.3f;
	std::uniform_real_distribution<float> distribution(-offset, offset);

	const float cellSizeInMeters = myCellSize * myResolution;
	const vec2i& tileIndex = aCell->GetGridIndex();
	const unsigned int numberOfInstancesPerSide = cellSizeInMeters * myDensityPerSqMeter;
	myGrassData.reserve(numberOfInstancesPerSide * numberOfInstancesPerSide);

	for (unsigned i = 0; i < numberOfInstancesPerSide; ++i)
	{
		for (unsigned j = 0; j < numberOfInstancesPerSide; ++j)
		{
			float x = (float)j / (float)myDensityPerSqMeter + tileIndex.x * cellSizeInMeters + distribution(myRandomEngine);
			float z = (float)i / (float)myDensityPerSqMeter + tileIndex.y * cellSizeInMeters + distribution(myRandomEngine);

			float y = aCell->GetY(x, z);

			GrassInstance grassInstance;
			grassInstance.x = x;
			grassInstance.y = y;
			grassInstance.z = z;

			auto norm = aCell->GetNormal(x, z);

			if (norm.x < 0.8f)
			{
				continue;
			}

			grassInstance.nx8 = norm.x * 128 + 128;
			grassInstance.ny8 = norm.y * 128 + 128;
			grassInstance.nz8 = norm.z * 128 + 128;

			myGrassData.push_back(grassInstance);
		}
	}
}

void Grass::Draw(const Manager::ShaderManager* aShaderManager, const vec2i& aTileIndex)
{
	++locElapsedTime; // TIME SYSTEM UPDATED IN SCENE MANAGER
	glDisable(GL_CULL_FACE);
	auto grassProgram = aShaderManager->GetShader("grassShader");
	glUseProgram(grassProgram);

	// we calculated those uniforms before
	GLuint tileIndexID = glGetUniformLocation(grassProgram, "tileIndex");
	glUniform2i(tileIndexID, aTileIndex.x, aTileIndex.y);

	GLuint cellSizeID = glGetUniformLocation(grassProgram, "cellSize");
	glUniform1i(cellSizeID, myCellSize);

	GLuint cellResolutionID = glGetUniformLocation(grassProgram, "resolution");
	glUniform1f(cellResolutionID, myResolution);

	GLuint elapsedTime = glGetUniformLocation(grassProgram, "elapsedTime");
	glUniform1f(elapsedTime, locElapsedTime);
	glUniform1i(glGetUniformLocation(grassProgram, "grassMaterial.diffuse"), 4);
	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, textures[4].myId);
	glBindVertexArray(VAOs[0]); // CREATE BUFFERS
	glDrawArrays(GL_POINTS, 0, cellSize * cellSize);
	glBindVertexArray(0);
	glEnable(GL_CULL_FACE);
}

void Grass::Reset()
{
	myGrassData.erase(myGrassData.begin(), myGrassData.end());
}
