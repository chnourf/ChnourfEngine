#include "TerrainTileBuilder.h"
#include"TerrainTile.h"
#include"TerrainGenerationFunctions.h"
#include <iostream>
#include "../Core/Vector.h"
#include "../WorldGenerator/TerrainManager.h"
#include "../Dependencies/imgui/imgui.h"

TerrainTileBuildingTask::TerrainTileBuildingTask(const int aSeed, const unsigned int aTileSize, float aTileResolution, TerrainTile* anEmptyTile):
	myTileSize(aTileSize),
	myTileResolution(aTileResolution)
{
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
			const float x = ((float)i / ((float)myTileSize-1) + aTile->GetGridIndex().x) * myTileSize * myTileResolution;
			const float y = ((float)j / ((float)myTileSize-1) + aTile->GetGridIndex().y) * myTileSize * myTileResolution;

			const auto pointNoise = TerrainGeneration::ComputeElevation(x, y, true);

			if (pointNoise < minHeight)
			{
				minHeight = pointNoise;
			}
			if (pointNoise > maxHeight)
			{
				maxHeight = pointNoise;
			}

			temporaryElements.push_back(TerrainElement(pointNoise, glm::vec3(), 255 * Manager::TerrainManager::GetInstance()->SampleRainfallFromGrid(vec2f(x, y)) , 255 * TerrainGeneration::ComputeTemperature(x, pointNoise, y), 0));
		}
	}
	aTile->myHeightmapBuildTime = ImGui::GetTime() - beforeElevationTime;

	aTile->SetMinHeight(minHeight);
	aTile->SetMaxHeight(maxHeight);

	std::vector<TerrainElement> elementsBeforeErosion = temporaryElements;

	auto timeBeforeErosion = ImGui::GetTime();
	//computing erosion, could be moved to presets.txt
	TerrainGeneration::ErosionParams params;
	params.carryCapacity = locCarryCapacity;
	params.iterations = locIterations;
	params.rockHardness = locRockHardness;
	params.depositionRadius = locErosionRadius;
	params.evaporation = locEvaporation;
	params.gravity = locGravity;
	params.depositionSpeed = locDepositionSpeed;
	TerrainGeneration::ComputeErosion(temporaryElements, params, myTileSize);
	aTile->myErosionBuildTime = ImGui::GetTime() - timeBeforeErosion;

	const float erosionStrength = 0.7f;
	const float lerpStrength = 3.f;
	// lerping the edges of the tiles to ensure continuity after erosion
	for (unsigned int i = 0; i < myTileSize; ++i) {
		for (unsigned int j = 0; j < myTileSize; ++j) {
			auto index = i + j * myTileSize;
			auto lerpFactor = glm::clamp(lerpStrength * 0.95f - abs(2.f * lerpStrength * (float)i / (float)myTileSize - lerpStrength), 0.f, 1.f);
			lerpFactor *= glm::clamp(lerpStrength * 0.95f - abs(2.f * lerpStrength * (float)j / (float)myTileSize - lerpStrength), 0.f, 1.f);
			auto& el = temporaryElements[index];
			auto& bel = elementsBeforeErosion[index];
			el.myErodedCoefficient = bel.myErodedCoefficient + lerpFactor * (el.myErodedCoefficient - bel.myErodedCoefficient);
			lerpFactor = glm::clamp(lerpFactor, 0.f, erosionStrength);
			el.myElevation = bel.myElevation + lerpFactor * (el.myElevation - bel.myElevation);
		}
	}

	//computing normals based on elevation
	for (unsigned int i = 0; i < myTileSize; ++i) {     // y
		for (unsigned int j = 0; j < myTileSize; ++j) {  // x

			auto delta = 1.f / ((float)myTileSize - 1);
			const float x = ((float)i * delta + aTile->GetGridIndex().x) * myTileSize * myTileResolution;
			const float y = ((float)j * delta + aTile->GetGridIndex().y) * myTileSize * myTileResolution;

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
	for (auto element : temporaryElements)
	{
		aTile->AddTerrainElement(element);
	}

	aTile->myTotalBuildTime = ImGui::GetTime() - startTime;
	aTile->OnFinishBuild();
}

TerrainTileBuildingTask::~TerrainTileBuildingTask()
{
}

TerrainTileBuilder::TerrainTileBuilder(const int aTileSize, const float aResolution, unsigned int aSeed):
	myTileSize(aTileSize),
	myTileResolution(aResolution),
	mySeed(aSeed)
{
}

void TerrainTileBuilder::BuildTileRequest(TerrainTile* aTile)
{
	if (myLoadingTasks.size() < myMaximumThreadLoad)
	{
		myLoadingTasks.push_back(new TerrainTileBuildingTask(mySeed, myTileSize, myTileResolution, aTile));
	}
	else
	{
		myLoadingQueue.push(aTile);
	}
}

void TerrainTileBuilder::Update()
{
	ImGui::SliderFloat("carry capacity : ", &locCarryCapacity, 0.f, 100.f, "%.3f", 2.f);
	ImGui::SliderFloat("rock Hardness : ", &locRockHardness, 0.f, 1.f, "%.3f", 2.f);
	ImGui::SliderFloat("deposition speed : ", &locDepositionSpeed, 0.f, 1.f, "%.3f", 2.f);
	ImGui::SliderInt("iterations : ", &locIterations, 1000, 200000);
	ImGui::SliderInt("Erosion Radius : ", &locErosionRadius, 0, 10);
	ImGui::SliderFloat("gravity : ", &locGravity, 0.f, 1.f, "%.3f", 2.f);
	ImGui::SliderFloat("evaportaion : ", &locEvaporation, 0.f, 1.f, "%.3f", 3.f);

	auto it = myLoadingTasks.begin();
	while (it < myLoadingTasks.end())
	{
		if ((*it)->myHandle.wait_for(std::chrono::seconds(0)) == std::future_status::ready)
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
			myLoadingQueue.pop();
		}
	}
}
