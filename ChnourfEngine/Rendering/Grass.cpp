#include "Grass.h"

#include "../WorldGenerator/TerrainCell.h"
#include "../Managers/ShaderManager.h"
#include "../Core/Math.h"
#include "../Core/Time.h"



#include <chrono>


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
	std::uniform_real_distribution<float> distribution(-1.f, 1.f);

	const float cellSizeInMeters = myCellSize * myResolution;
	const vec2i& tileIndex = aCell->GetGridIndex();
	const unsigned int numberOfInstancesPerSide = cellSizeInMeters * myDensityPerSqMeter;
	myGrassData.reserve(numberOfInstancesPerSide * numberOfInstancesPerSide);

	auto upperLimit = (float)(myCellSize - 1) * myResolution;

	for (unsigned i = 0; i < numberOfInstancesPerSide; ++i)
	{
		for (unsigned j = 0; j < numberOfInstancesPerSide; ++j)
		{
			float x = tileIndex.x * cellSizeInMeters + glm::clamp((float)j / (float)myDensityPerSqMeter + offset * distribution(myRandomEngine), 0.f, upperLimit);
			float z = tileIndex.y * cellSizeInMeters + glm::clamp((float)i / (float)myDensityPerSqMeter + offset * distribution(myRandomEngine), 0.f, upperLimit);

			float y = aCell->GetElement(j/2 * myCellSize + i/2).myElevation;
			//float y = aCell->GetY(x, z);

			GrassInstance grassInstance;
			grassInstance.x = x;
			grassInstance.y = y;
			grassInstance.z = z;

			auto norm = aCell->GetElement(j / 2 * myCellSize + i / 2).myNormal;// aCell->GetNormal(x, z);

			if (norm.y < 0.8f)
			{
				continue;
			}

			grassInstance.nx8 = norm.x * 128 + 128;
			grassInstance.ny8 = norm.y * 128 + 128;
			grassInstance.nz8 = norm.z * 128 + 128;

			grassInstance.atlasIndex8 = 0;
			grassInstance.colorLerp8 = 0;

			auto scale = 0.8f + 0.2f * distribution(myRandomEngine);
			grassInstance.scale8 = scale * 128 + 128;

			auto direction = distribution(myRandomEngine);
			grassInstance.direction8 = direction * 128 + 128;

			myGrassData.push_back(grassInstance);
		}
	}

	glGenBuffers(1, &myInstanceVBO);
	glBindBuffer(GL_ARRAY_BUFFER, myInstanceVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GrassInstance) * myGrassData.size(), &myGrassData[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	float quadVertices[] = {
		// positions
		-0.5f,  0.5f,
		0.5f, -0.5f,
		-0.5f, -0.5f,

		-0.5f,  0.5f,
		0.5f, -0.5f,
		0.5f,  0.5f,
	};

	glGenVertexArrays(1, &myVAO);
	glGenBuffers(1, &myVBO);

	glBindVertexArray(myVAO);
	glBindBuffer(GL_ARRAY_BUFFER, myVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices[0], GL_STATIC_DRAW);

	// Quad Positions
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);

	glBindBuffer(GL_ARRAY_BUFFER, myInstanceVBO);
	// Instances Positions
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(GrassInstance), (void*)0);
	glVertexAttribDivisor(1, 1);

	// Normal
	glEnableVertexAttribArray(2);
	glVertexAttribIPointer(2, 4, GL_UNSIGNED_BYTE, sizeof(GrassInstance), (GLvoid*)offsetof(GrassInstance, nx8));
	glVertexAttribDivisor(2, 1);

	// Misc
	glEnableVertexAttribArray(3);
	glVertexAttribIPointer(3, 4, GL_UNSIGNED_BYTE, sizeof(GrassInstance), (GLvoid*)offsetof(GrassInstance, atlasIndex8));
	glVertexAttribDivisor(3, 1);

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void Grass::Draw(const Manager::ShaderManager* aShaderManager, const vec2i& aTileIndex, GLuint aGrassTexture)
{
	if (myGrassData.size() == 0)
	{
		return;
	}

	glDisable(GL_CULL_FACE);
	auto grassProgram = aShaderManager->GetShader("grassShader");
	glUseProgram(grassProgram);

	GLuint elapsedTime = glGetUniformLocation(grassProgram, "elapsedTime");

	std::chrono::milliseconds ms = std::chrono::duration_cast< std::chrono::milliseconds >(
		std::chrono::system_clock::now().time_since_epoch()
		);

	glUniform1f(elapsedTime, (float) ms.count()); // TEMPORARY
	glUniform1i(glGetUniformLocation(grassProgram, "grassMaterial.diffuse"), 4);
	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, aGrassTexture);
	glBindVertexArray(myVAO);
	glDrawArraysInstanced(GL_TRIANGLES, 0, 6, myGrassData.size());
	glBindVertexArray(0);
	glEnable(GL_CULL_FACE);
}

void Grass::Reset()
{
	myGrassData.erase(myGrassData.begin(), myGrassData.end());
	glDeleteBuffers(1, &myVBO);
}
