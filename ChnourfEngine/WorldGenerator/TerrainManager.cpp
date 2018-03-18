#pragma once
#include <algorithm>
#include "../Managers/SceneManager.h"
#include "../Managers/ModelManager.h"
#include "TerrainManager.h"
#include "TerrainTile.h"

#include "../Dependencies/imgui/imgui.h"

#include "TerrainGenerationFunctions.h"

#include "WorldGrid.h"

#include <array>
#include <iostream>
#include <fstream>
#include <sstream>

namespace Manager
{
	TerrainManager::TerrainManager() :
		myTileSize{ 0 },
		myResolution{ 0 },
		myDetectionRadius{ 0 }
	{
		std::ifstream file("Data/TerrainGenerator/Presets.txt");
		if (file)
		{
			std::stringstream ss;

			ss << file.rdbuf();

			std::string field;
			float value;

			while (ss >> field >> value)
			{
				if (strcmp(field.c_str(), "DetectionRadius") == 0)
				{
					myDetectionRadius = unsigned(value);
					myCachedRadius = unsigned(value + 1);
				}
				else if (strcmp(field.c_str(), "TileSize") == 0)
				{
					myTileSize = unsigned(value);
				}
				else if (strcmp(field.c_str(), "TileResolution") == 0)
				{
					myResolution = value;
				}
			}
		}

		file.close();

		mySeed = time(NULL);
		srand(mySeed);

		TerrainGeneration::Initialize(mySeed);

		auto mustBeAPowerOf2 = myTileSize - 1;
		assert(((mustBeAPowerOf2 != 0) && ((mustBeAPowerOf2 & (~mustBeAPowerOf2 + 1)) == mustBeAPowerOf2)));
		myTileBuilder = std::make_unique<TerrainTileBuilder>(myTileSize, myResolution, mySeed);
		myWorldGrid = std::make_unique<TerrainGeneration::WorldGrid> (mySeed);

		myWorldGrid->Generate();
	}

	TerrainManager::~TerrainManager()
	{
		for (auto tile : myActiveTiles)
		{
			delete tile;
		}
	}

	static const auto midPos = Vector2<float>(TerrainGeneration::GetMapSize() / 2.f, TerrainGeneration::GetMapSize() / 2.f);

	void TerrainManager::Update(const vec3f& aPlayerPosition)
	{
		ImGui::Begin("Terrain Generation");
		if (ImGui::Button("Invalidate Tiles"))
		{
			auto activeTilesIt = myActiveTiles.begin();
			while (activeTilesIt < myActiveTiles.end())
			{
				const vec2i tileIndex = (*activeTilesIt)->GetGridIndex();
				SceneManager::GetInstance()->GetModelManager()->RemoveTerrainTile(tileIndex);
				delete *activeTilesIt;
				activeTilesIt = myActiveTiles.erase(activeTilesIt);
			}
		}
		ImGui::Text("Tiles loaded : %d", myActiveTiles.size());
		ImGui::Text("Tiles loading : %d", myTilesToLoad.size());
		auto averageTotalTime = std::accumulate(myActiveTiles.begin(), myActiveTiles.end(), 0.f, [](float a, const TerrainTile* b) {return a + b->myTotalBuildTime; });
		auto averageErosionTime = std::accumulate(myActiveTiles.begin(), myActiveTiles.end(), 0.f, [](float a, const TerrainTile* b) {return a + b->myErosionBuildTime; });
		auto averageHeightmapTime = std::accumulate(myActiveTiles.begin(), myActiveTiles.end(), 0.f, [](float a, const TerrainTile* b) {return a + b->myHeightmapBuildTime; });
		if (myActiveTiles.size() > 0)
		{
			averageTotalTime /= float(myActiveTiles.size());
			averageErosionTime /= float(myActiveTiles.size());
			averageHeightmapTime /= float(myActiveTiles.size());
			ImGui::Text("Avg Tile loading time : %f ms", 1000.f*averageTotalTime);
			ImGui::Text("Avg Tile erosion computing time : %f ms", 1000.f*averageErosionTime);
			ImGui::Text("Avg Tile heightmap/temperature/rainfall time : %f ms", 1000.f*averageHeightmapTime);
		}
		ImGui::End();

		vec2f adjustedPostion = vec2f(aPlayerPosition.x + TerrainGeneration::GetMapSize() / 2.f, aPlayerPosition.z + TerrainGeneration::GetMapSize() / 2.f);
		ImGui::Text("Element on Grid : x %d, y %d", int(adjustedPostion.x)*TerrainGeneration::GetMapTileAmount() / int(TerrainGeneration::GetMapSize()), int(adjustedPostion.y)*TerrainGeneration::GetMapTileAmount() / int(TerrainGeneration::GetMapSize()));

		auto temperature = TerrainGeneration::ComputeTemperature(aPlayerPosition.x, aPlayerPosition.y, aPlayerPosition.z);
		auto temperatureInCelsius = int(70.f * temperature - 30.f);
		ImGui::Text("Temperature at camera position : %03f, or %d Celsius", temperature, temperatureInCelsius);

		auto rainfall = TerrainGeneration::ComputeRainfallFromGridWithPerlinNoise(aPlayerPosition.x, aPlayerPosition.z);
		ImGui::Text("Rainfall at camera position : %03f", rainfall);

		ImGui::Text("Current Biome : %s", TerrainGeneration::GetBiomeName(TerrainGeneration::DeduceBiome(temperature, rainfall)));

		const vec2i positionOnGrid = vec2i(aPlayerPosition.x / (myTileSize*myResolution), aPlayerPosition.z / (myTileSize*myResolution));

		// detection square
		const int minX = positionOnGrid.x - myDetectionRadius;
		const int maxX = positionOnGrid.x + myDetectionRadius;
		const int minY = positionOnGrid.y - myDetectionRadius;
		const int maxY = positionOnGrid.y + myDetectionRadius;

		const auto squaredDetectionRadius = pow(myDetectionRadius, 2);

		for (int x = minX; x < maxX; x++)
		{
			for (int y = minY; y < maxY; y++)
			{
				const auto tile = vec2i(x, y);

				// detection circle
				if (pow(x - positionOnGrid.x, 2) + pow(y - positionOnGrid.y, 2) > squaredDetectionRadius)
				{
					continue;
				}
				
				if (!IsTileLoaded(tile) && !IsTileLoading(tile))
				{
					LoadTile(vec2i(x, y));
				}
			}
		}

		const auto squaredCacheRadius = pow(myCachedRadius, 2);

		auto loadingTilesIt = myTilesToLoad.begin();
		while (loadingTilesIt < myTilesToLoad.end())
		{
			if ((*loadingTilesIt)->IsBuilt())
			{
				myActiveTiles.push_back(*loadingTilesIt);
				SceneManager::GetInstance()->GetModelManager()->AddTerrainTile(*loadingTilesIt, myTileSize, myResolution);

				std::iter_swap(loadingTilesIt, myTilesToLoad.end() - 1);
				myTilesToLoad.pop_back();
			}
			else
			{
				const vec2i tileIndex = (*loadingTilesIt)->GetGridIndex();
				if (pow(tileIndex.x - positionOnGrid.x, 2) + pow(tileIndex.y - positionOnGrid.y, 2) > squaredCacheRadius && !(*loadingTilesIt)->IsBuilding())
				{
					myTileBuilder->CancelBuildRequest(*loadingTilesIt);
					delete *loadingTilesIt;
					std::iter_swap(loadingTilesIt, myTilesToLoad.end() - 1);
					myTilesToLoad.pop_back();
				}
				else
				{
					++loadingTilesIt;
				}
			}
		}

		auto activeTilesIt = myActiveTiles.begin();
		while (activeTilesIt < myActiveTiles.end())
		{
			const vec2i tileIndex = (*activeTilesIt)->GetGridIndex();
			if (pow(tileIndex.x - positionOnGrid.x, 2) + pow(tileIndex.y - positionOnGrid.y, 2) > squaredCacheRadius)
			{
				SceneManager::GetInstance()->GetModelManager()->RemoveTerrainTile(tileIndex);
				delete *activeTilesIt;
				std::iter_swap(activeTilesIt, myActiveTiles.end() - 1);
				myActiveTiles.pop_back();
			}
			else
			{
				++activeTilesIt;
			}
		}

		myTileBuilder->Update();
	}

	bool TerrainManager::IsTileLoaded(const vec2i& aTileIndex) const
	{
		return std::find_if(myActiveTiles.begin(), myActiveTiles.end(),
			[aTileIndex](const TerrainTile* aLoadedTile)
		{return aLoadedTile->GetGridIndex() == aTileIndex; })
			!= myActiveTiles.end();
	}

	bool TerrainManager::IsTileLoading(const vec2i& aTileIndex) const
	{
		return std::find_if(myTilesToLoad.begin(), myTilesToLoad.end(),
			[aTileIndex](TerrainTile* aTileToLoad) 
		{return aTileToLoad->GetGridIndex() == aTileIndex; })
			!= myTilesToLoad.end();
	}

	void TerrainManager::LoadTile(const vec2i& aGridIndex)
	{
		auto tile = new TerrainTile(aGridIndex, myTileSize, myResolution);
		myTilesToLoad.push_back(tile);
		myTileBuilder->BuildTileRequest(tile);
	}
}