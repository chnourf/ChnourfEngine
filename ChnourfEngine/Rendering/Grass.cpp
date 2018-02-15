#include "Grass.h"

#include "../WorldGenerator/TerrainTile.h"
#include "../Managers/ShaderManager.h"
#include "../Core/Math.h"
#include "../Core/Time.h"

#include "../WorldGenerator/TerrainGenerationFunctions.h"
#include "../WorldGenerator/TerrainManager.h"


const float locGrassMaxAltitude = 380.f;
const float locGrassAltRandomAmplitude = 40.f;
const float locGrassTotalMaxAltitude = locGrassMaxAltitude + locGrassAltRandomAmplitude;

Grass::Grass(unsigned int aTileSize, float aResolution, int aSeed):
	myTileSize(aTileSize),
	myResolution(aResolution),
	mySeed(aSeed),
	myIsGenerated(false)
{
	myGeneratingTask = std::future<void>();
	myDensityPerSqMeter = 2;
}

Grass::~Grass()
{
	// OPTIMIZATION : KILL THE TASK EVEN IF NOT FINISHED
	if (myGeneratingTask.valid())
	{
		myGeneratingTask.wait();
	}
	myGrassData.clear();
}

void Grass::GenerateGrass(const TerrainTile* aTile)
{
	if (myIsGenerated)
	{
		return;
	}

	// no need to generate in that case, the tile is too high, or below sea level
	if (aTile->GetMinHeight() > locGrassTotalMaxAltitude || aTile->GetMaxHeight() < TerrainGeneration::seaLevel)
	{
		return;
	}

	myIsGenerating = true;

	// resetting engine
	myRandomEngine.seed(mySeed);
	const float offset = 0.3f;
	std::uniform_real_distribution<float> distribution(-1.f, 1.f);

	const float tileSizeInMeters = myTileSize * myResolution;
	const vec2i& tileIndex = aTile->GetGridIndex();
	const unsigned int numberOfInstancesPerSide = tileSizeInMeters * myDensityPerSqMeter;
	//myGrassData.reserve(numberOfInstancesPerSide * numberOfInstancesPerSide);

	auto upperLimit = (float)(myTileSize - 1) * myResolution;

	const auto multiplier = (myDensityPerSqMeter*myResolution);
	for (unsigned i = 0; i < numberOfInstancesPerSide; ++i)
	{
		for (unsigned j = 0; j < numberOfInstancesPerSide; ++j)
		{
			float x = tileIndex.x * tileSizeInMeters + glm::clamp((float)j / (float)myDensityPerSqMeter + offset * distribution(myRandomEngine), 0.f, upperLimit);
			float z = tileIndex.y * tileSizeInMeters + glm::clamp((float)i / (float)myDensityPerSqMeter + offset * distribution(myRandomEngine), 0.f, upperLimit);

			float y = aTile->GetY(x, z);

			if (y + distribution(myRandomEngine) * locGrassAltRandomAmplitude > locGrassMaxAltitude)
			{
				continue;
			}

			if (y < TerrainGeneration::seaLevel)
			{
				continue;
			}

			auto elementRainfall = aTile->GetElement(floor(j / multiplier) * myTileSize + floor(i / multiplier)).myRainfall;
			if (float(elementRainfall)/255.f < (0.3f + 0.2f * distribution(myRandomEngine)))
			{
				continue;
			}

			auto norm = aTile->GetElement(floor(j / multiplier) * myTileSize + floor(i / multiplier)).myNormal;// aTile->GetNormal(x, z);

			if (norm.y < 0.85f)
			{
				continue;
			}

			GrassInstance grassInstance;
			grassInstance.x = x;
			grassInstance.y = y;
			grassInstance.z = z;

			grassInstance.nx8 = norm.x * 128 + 127;
			grassInstance.ny8 = norm.y * 128 + 127;
			grassInstance.nz8 = norm.z * 128 + 127;

			grassInstance.atlasIndex8 = 0;

			auto scale = 0.7f + 0.3f * distribution(myRandomEngine);
			grassInstance.scale8 = scale * 128 + 127;

			auto direction = distribution(myRandomEngine);
			grassInstance.direction8 = direction * 128 + 127;

			grassInstance.rainfall8 = elementRainfall;
			grassInstance.temperature8 = aTile->GetElement(floor(j / multiplier) * myTileSize + floor(i / multiplier)).myTemperature;

			myGrassData.push_back(grassInstance);
		}
	}
}

void Grass::Update(bool aMustGenerate, const TerrainTile* aTile)
{
	if (aMustGenerate)
	{
		if (!myIsGenerated)
		{
			if (!myIsGenerating)
			{
				myGeneratingTask = std::async(std::launch::async, [this, aTile]() { GenerateGrass(aTile); });
			}
			else if (myGeneratingTask.wait_for(std::chrono::seconds(0)) == std::future_status::ready)
			{
				OnGrassGenerationComplete();
			}
		}
	}
	else
	{
		Reset();
	}
}

void Grass::Draw(const Manager::ShaderManager* aShaderManager, const vec2i& aTileIndex, GLuint aGrassTexture, GLuint aGrassColorTexture)
{
	//Use special mipmap for texture with alpha ? Or alpha blending

	if (myGrassData.size() == 0)
	{
		return;
	}

	glDisable(GL_CULL_FACE);
	auto grassProgram = aShaderManager->GetShader("grassShader");
	glUseProgram(grassProgram);

	GLuint elapsedTimeID = glGetUniformLocation(grassProgram, "elapsedTime");
	float time = Time::GetInstance()->GetTime();
	glUniform1f(elapsedTimeID, time);

	glUniform1i(glGetUniformLocation(grassProgram, "grassMaterial.diffuse"), 5);
	glActiveTexture(GL_TEXTURE5);
	glBindTexture(GL_TEXTURE_2D, aGrassTexture);

	glUniform1i(glGetUniformLocation(grassProgram, "grassColorTexture"), 8);
	glActiveTexture(GL_TEXTURE8);
	glBindTexture(GL_TEXTURE_2D, aGrassColorTexture);

	glBindVertexArray(myVAO);
	glDrawArraysInstanced(GL_TRIANGLES, 0, 6, myGrassData.size());
	glBindVertexArray(0);
	glEnable(GL_CULL_FACE);
}

void Grass::Reset()
{
	myIsGenerated = false;
	myIsGenerating = false;
	myGrassData.clear();
	glDeleteBuffers(1, &myVBO);
}

void Grass::OnGrassGenerationComplete()
{
	if (myGrassData.size() == 0)
	{
		return;
	}

	glGenBuffers(1, &myInstanceVBO);
	glBindBuffer(GL_ARRAY_BUFFER, myInstanceVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GrassInstance) * myGrassData.size(), &myGrassData[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	float quadVertices[] = {
		// positions
		-0.5f,  1.f,
		0.5f, 0.f,
		-0.5f, 0.f,

		-0.5f,  1.f,
		0.5f, 0.f,
		0.5f,  1.f,
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
	glVertexAttribIPointer(3, 4, GL_UNSIGNED_BYTE, sizeof(GrassInstance), (GLvoid*)offsetof(GrassInstance, direction8));
	glVertexAttribDivisor(3, 1);

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	myIsGenerated = true;
	myIsGenerating = false;
}
