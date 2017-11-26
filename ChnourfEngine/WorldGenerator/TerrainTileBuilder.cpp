#include "TerrainTileBuilder.h"
#include"TerrainTile.h"
#include"TerrainGenerationFunctions.h"
#include <iostream>
#include "../Core/Vector.h"
#include "../WorldGenerator/TerrainManager.h"

TerrainTileBuildingTask::TerrainTileBuildingTask(const int aSeed, const unsigned int aTileSize, float aTileResolution, TerrainTile* anEmptyTile):
	myTileSize(aTileSize),
	myTileResolution(aTileResolution)
{
	myPerlin = PerlinNoise(aSeed);
	myRandomEngine.seed(aSeed);
	myHandle = std::async(std::launch::async, [this, anEmptyTile]() {BuildTile(anEmptyTile);});
}

void TerrainTileBuildingTask::BuildTile(TerrainTile* aTile)
{
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
	for (unsigned int i = 0; i < myTileSize; ++i) {     // y
		for (unsigned int j = 0; j < myTileSize; ++j) {  // x
			const float x = ((float)j / ((float)myTileSize-1) + aTile->GetGridIndex().y) * myTileSize * myTileResolution;
			const float y = ((float)i / ((float)myTileSize-1) + aTile->GetGridIndex().x) * myTileSize * myTileResolution;

			const auto pointNoise = TerrainGeneration::ComputeElevation(x, y, myPerlin, true);

			if (pointNoise < minHeight)
			{
				minHeight = pointNoise;
			}
			if (pointNoise > maxHeight)
			{
				maxHeight = pointNoise;
			}

			temporaryElements.push_back(TerrainElement(pointNoise, glm::vec3()));
		}
	}

	aTile->SetMinHeight(minHeight);
	aTile->SetMaxHeight(maxHeight);

	//std::vector<TerrainElement> elementsBeforeErosion = temporaryElements;

	////computing erosion, could be moved to presets.txt
	//TerrainGeneration::ErosionParams params;
	//params.Kq = 1.5f;
	//params.Kevap = 0.1f;
	//params.Kerosion = .9f;
	//params.Kdepos = .02f;
	//params.Ki = .01f;
	//params.minSlope = 0.05f;
	//params.g = 1.f;
	//TerrainGeneration::ComputeErosion(temporaryElements, 10000, params, myTileSize, myRandomEngine);

	//// lerping the edges of the tiles to ensure continuity
	//for (unsigned int i = 0; i < myTileSize; ++i) {
	//	for (unsigned int j = 0; j < myTileSize; ++j) {

	//		auto index = i + j * myTileSize;
	//		auto lerpFactor = glm::clamp(5.5f - abs(12.f * (float)i / (float)myTileSize - 6.f), 0.f, 1.f);
	//		lerpFactor *= glm::clamp(5.5f - abs(12.f * (float)j / (float)myTileSize - 6.f), 0.f, 1.f);
	//		auto& el = temporaryElements[index].myElevation;
	//		auto& bel = elementsBeforeErosion[index].myElevation;
	//		el = bel + lerpFactor * (el - bel);
	//	}
	//}

	//computing normals based on elevation
	for (unsigned int i = 0; i < myTileSize; ++i) {     // y
		for (unsigned int j = 0; j < myTileSize; ++j) {  // x

			auto delta = 1.f / ((float)myTileSize - 1);
			const float x = ((float)j * delta + aTile->GetGridIndex().y) * myTileSize * myTileResolution;
			const float y = ((float)i * delta + aTile->GetGridIndex().x) * myTileSize * myTileResolution;

			const auto index = j + i * myTileSize;

			// technically erosion should have changed the elevation of the edges but we suppose it's negligible, thanks to the lerp above
			const auto s01 = (j == 0) ? TerrainGeneration::ComputeElevation(x - delta * myTileSize * myTileResolution, y, myPerlin, true) : temporaryElements[index - 1].myElevation;
			const auto s21 = (j == myTileSize - 1) ? TerrainGeneration::ComputeElevation(x + delta * myTileSize * myTileResolution, y, myPerlin, true) : temporaryElements[index + 1].myElevation;
			const auto s10 = (i == 0) ? TerrainGeneration::ComputeElevation(x, y - delta * myTileSize * myTileResolution, myPerlin, true) : temporaryElements[index - myTileSize].myElevation;
			const auto s12 = (i == myTileSize - 1) ? TerrainGeneration::ComputeElevation(x, y + delta * myTileSize * myTileResolution, myPerlin, true) : temporaryElements[index + myTileSize].myElevation;
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
