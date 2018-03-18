#include "TerrainTileBuilder.h"
#include"TerrainTile.h"
#include"TerrainGenerationFunctions.h"
#include <iostream>
#include "../Core/Vector.h"
#include "../Managers/SceneManager.h"
#include "../WorldGenerator/TerrainManager.h"
#include "../Dependencies/imgui/imgui.h"
#include "../Dependencies/glew/glew.h"

TerrainTileBuildingTask::TerrainTileBuildingTask(const int aSeed, const unsigned int aTileSize, float aTileResolution, TerrainTile* anEmptyTile):
	myTileSize{ aTileSize },
	myTileResolution{ aTileResolution }
{
	assert(!anEmptyTile->IsBuilt() && !anEmptyTile->IsBuilding());
	myHandle = std::async(std::launch::async, [this, anEmptyTile]() {BuildTile(anEmptyTile);});
}

float locCarryCapacity = 5.f;
float locRockHardness = 0.3f;
float locDepositionSpeed = 0.1f;
int locIterations = 30000;
int locErosionRadius = 1;
float locGravity = 0.007f;
float locEvaporation = 0.01f;

void TerrainTileBuildingTask::BuildTile(TerrainTile* aTile)
{
	aTile->OnStartBuild();

	auto startTime = ImGui::GetTime();

	if (!aTile)
	{
		std::cout << "ERROR : tile given to build does not exist" << std::endl;
		return;
	}

	float minHeight = std::numeric_limits<float>::max();
	float maxHeight = std::numeric_limits<float>::min();

	std::vector<TerrainElement> temporaryElements;
	temporaryElements.reserve(myTileSize * myTileSize);

	//computing elevation
	auto beforeElevationTime = ImGui::GetTime();
	for (unsigned int i = 0; i < myTileSize; ++i) {     // y
		for (unsigned int j = 0; j < myTileSize; ++j) {  // x
			const float x = (float(i) / float(myTileSize-1) + aTile->GetGridIndex().x) * myTileSize * myTileResolution;
			const float y = (float(j) / float(myTileSize-1) + aTile->GetGridIndex().y) * myTileSize * myTileResolution;

			const auto pointNoise = TerrainGeneration::ComputeElevation(x, y, true);

			if (pointNoise < minHeight)
			{
				minHeight = pointNoise;
			}
			if (pointNoise > maxHeight)
			{
				maxHeight = pointNoise;
			}

			temporaryElements.push_back(TerrainElement{ pointNoise, glm::vec3(), unsigned char(255 * TerrainGeneration::ComputeRainfallFromGridWithPerlinNoise(x, y)),  unsigned char(255 * TerrainGeneration::ComputeTemperature(x, pointNoise, y)), 0 });
		}
	}
	aTile->myHeightmapBuildTime = ImGui::GetTime() - beforeElevationTime;

	aTile->SetMinHeight(minHeight);
	aTile->SetMaxHeight(maxHeight);

	std::vector<TerrainElement> elementsBeforeErosion = temporaryElements;

	//GLuint heightmapTextureID;
	//glGenTextures(1, &heightmapTextureID);
	//glBindTexture(GL_TEXTURE_2D, heightmapTextureID);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	//glGenerateMipmap(GL_TEXTURE_2D);
	//glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, myTileSize, myTileSize, 0, GL_R, GL_FLOAT, NULL);
	//glBindTexture(GL_TEXTURE_2D, 0);

	//auto computeErosionShader = Manager::SceneManager::GetInstance()->GetShaderManager()->GetShader("computeErosionShader");
	//glUseProgram(computeErosionShader);
	//glBindTexture(GL_TEXTURE_2D, heightmapTextureID);
	//glBindImageTexture(0, heightmapTextureID, 0, GL_FALSE, 0, GL_READ_WRITE, GL_R32F);
	//glDispatchCompute(1, 1, 1);
	//glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
	//glBindImageTexture(0, 0, 0, GL_FALSE, 0, GL_READ_WRITE, GL_R32F);
	//glBindTexture(GL_TEXTURE_2D, 0);
	//glUseProgram(0);

	auto timeBeforeErosion = ImGui::GetTime();
	//computing erosion, could be moved to presets.txt
	TerrainGeneration::ErosionParams params
	{
		locCarryCapacity,
		locRockHardness,
		locDepositionSpeed,
		locIterations,
		locErosionRadius,
		locGravity,
		locEvaporation,
	};

	if (maxHeight >= TerrainGeneration::seaLevel)
	{
		TerrainGeneration::ComputeErosion(temporaryElements, params, myTileSize);

		static const float erosionStrength = 0.7f;
		static const float lerpStrength = 3.f;
		// lerping the edges of the tiles to ensure continuity after erosion
		for (unsigned int i = 0; i < myTileSize; ++i) {
			for (unsigned int j = 0; j < myTileSize; ++j) {
				const auto index = i + j * myTileSize;
				auto lerpFactor = glm::clamp(lerpStrength * 0.95f - abs(2.f * lerpStrength * float(i) / float(myTileSize) - lerpStrength), 0.f, 1.f);
				lerpFactor *= glm::clamp(lerpStrength * 0.95f - abs(2.f * lerpStrength * float(j) / float(myTileSize) - lerpStrength), 0.f, 1.f);
				auto& el = temporaryElements[index];
				const auto& bel = elementsBeforeErosion[index];
				el.myErodedCoefficient = bel.myErodedCoefficient + lerpFactor * (el.myErodedCoefficient - bel.myErodedCoefficient);
				lerpFactor = glm::clamp(lerpFactor, 0.f, erosionStrength);
				el.myElevation = bel.myElevation + lerpFactor * (el.myElevation - bel.myElevation);
			}
		}
	}
	aTile->myErosionBuildTime = ImGui::GetTime() - timeBeforeErosion;

	//computing normals based on elevation
	for (unsigned int i = 0; i < myTileSize; ++i) {     // y
		for (unsigned int j = 0; j < myTileSize; ++j) {  // x

			static const auto delta = 1.f / (float(myTileSize) - 1);
			const float x = (float(i) * delta + aTile->GetGridIndex().x) * myTileSize * myTileResolution;
			const float y = (float(j) * delta + aTile->GetGridIndex().y) * myTileSize * myTileResolution;

			const auto index = j + i * myTileSize;

			// technically erosion should have changed the elevation of the edges but we suppose it's negligible, thanks to the lerp above
			const auto s01 = (j == 0) ? TerrainGeneration::ComputeElevation(x, y - delta * myTileSize * myTileResolution, true) : temporaryElements[index - 1].myElevation;
			const auto s21 = (j == myTileSize - 1) ? TerrainGeneration::ComputeElevation(x, y + delta * myTileSize * myTileResolution, true) : temporaryElements[index + 1].myElevation;
			const auto s10 = (i == 0) ? TerrainGeneration::ComputeElevation(x - delta * myTileSize * myTileResolution, y, true) : temporaryElements[index - myTileSize].myElevation;
			const auto s12 = (i == myTileSize - 1) ? TerrainGeneration::ComputeElevation(x + delta * myTileSize * myTileResolution, y, true) : temporaryElements[index + myTileSize].myElevation;
			const glm::vec3 va = glm::normalize(glm::vec3(2 * delta, (s21 - s01) / (myTileSize * myTileResolution), 0.0f));
			const glm::vec3 vb = glm::normalize(glm::vec3(0.0f, (s12 - s10) / (myTileSize * myTileResolution), 2 * delta));
			const auto normal = glm::cross(vb, va);

			temporaryElements[j + i * myTileSize].myNormal = normal;
		}
	}

	// should not take long as size was already reserved
	for (const auto& element : temporaryElements)
	{
		aTile->AddTerrainElement(element);
	}

	aTile->myTotalBuildTime = ImGui::GetTime() - startTime;
	aTile->OnFinishBuild();
}

TerrainTileBuildingTask::~TerrainTileBuildingTask()
{}

TerrainTileBuilder::TerrainTileBuilder(const int aTileSize, const float aResolution, unsigned int aSeed):
	myTileSize{ unsigned(aTileSize) },
	myTileResolution{ aResolution },
	mySeed{ aSeed }
{}

void TerrainTileBuilder::BuildTileRequest(TerrainTile* aTile)
{
	if (myLoadingTasks.size() < myMaximumThreadLoad)
	{
		myLoadingTasks.push_back(new TerrainTileBuildingTask(mySeed, myTileSize, myTileResolution, aTile));
	}
	else
	{
		myLoadingQueue.push_back(aTile);
	}
}

void TerrainTileBuilder::CancelBuildRequest(const TerrainTile * aTile)
{
	myLoadingQueue.erase(std::remove(myLoadingQueue.begin(), myLoadingQueue.end(), aTile));
}

void TerrainTileBuilder::Update()
{
	ImGui::SliderFloat("Carry capacity : ", &locCarryCapacity, 0.f, 100.f, "%.3f", 2.f);
	ImGui::SliderFloat("Rock Hardness : ", &locRockHardness, 0.f, 1.f, "%.3f", 2.f);
	ImGui::SliderFloat("Deposition speed : ", &locDepositionSpeed, 0.f, 1.f, "%.3f", 2.f);
	ImGui::SliderInt("Iterations : ", &locIterations, 0, 200000);
	ImGui::SliderInt("Erosion Radius : ", &locErosionRadius, 0, 10);
	ImGui::SliderFloat("Gravity : ", &locGravity, 0.f, 1.f, "%.3f", 2.f);
	ImGui::SliderFloat("Evaportaion : ", &locEvaporation, 0.f, 1.f, "%.3f", 3.f);

	auto it = myLoadingTasks.begin();
	while (it < myLoadingTasks.end())
	{
		static auto waitTime{ std::chrono::seconds(0) };
		if ((*it)->myHandle.wait_for(waitTime) == std::future_status::ready)
		{
			delete *it;
			it = myLoadingTasks.erase(it);
		}
		else
		{
			++it;
		}
	}

	for (int i = 0; i < myMaximumThreadLoad - myLoadingTasks.size(); ++i)
	{
		if (!myLoadingQueue.empty())
		{
			myLoadingTasks.push_back(new TerrainTileBuildingTask(mySeed, myTileSize, myTileResolution, myLoadingQueue.front()));
			myLoadingQueue.pop_front();
		}
	}
}
