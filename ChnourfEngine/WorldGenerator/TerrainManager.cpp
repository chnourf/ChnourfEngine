#pragma once
#include <algorithm>
#include "../Managers/SceneManager.h"
#include "../Managers/ModelManager.h"
#include "TerrainManager.h"
#include "TerrainTile.h"

#include "TerrainGenerationFunctions.h"

#include "WorldGrid.h"

#include <array>
#include <iostream>
#include <fstream>
#include <sstream>

namespace Manager
{
	TerrainManager::TerrainManager() :
		myTileSize(0),
		myResolution(0),
		myDetectionRadius(0)
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
					myDetectionRadius = (unsigned int) value;
					myCachedRadius = (unsigned int) (value + 1);
				}
				else if (strcmp(field.c_str(), "TileSize") == 0)
				{
					myTileSize = (unsigned int) value;
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

		auto mustBeAPowerOf2 = myTileSize - 1;
		assert(((mustBeAPowerOf2 != 0) && ((mustBeAPowerOf2 & (~mustBeAPowerOf2 + 1)) == mustBeAPowerOf2)));
		myTileBuilder = std::make_unique<TerrainTileBuilder>(myTileSize, myResolution, mySeed);
		myWorldGrid = std::make_unique<TerrainGeneration::WorldGrid> (mySeed);

		myWorldGrid.get()->Generate();
	}

	TerrainManager::~TerrainManager()
	{
		for (auto tile : myActiveTiles)
		{
			delete tile;
		}
	}

	void TerrainManager::Update(const vec3f& aPlayerPosition)
	{
		myTileBuilder->Update();

		const vec2i positionOnGrid = vec2i(aPlayerPosition.x / (myTileSize*myResolution), aPlayerPosition.z / (myTileSize*myResolution));

		// detection square
		const int minX = positionOnGrid.x - myDetectionRadius;
		const int maxX = positionOnGrid.x + myDetectionRadius;
		const int minY = positionOnGrid.y - myDetectionRadius;
		const int maxY = positionOnGrid.y + myDetectionRadius;
		for (int x = minX; x < maxX; x++)
		{
			for (int y = minY; y < maxY; y++)
			{
				auto tile = vec2i(x, y);

				// detection circle
				if (pow(x - positionOnGrid.x, 2) + pow(y - positionOnGrid.y, 2) > pow(myDetectionRadius, 2))
				{
					continue;
				}
				
				if (!IsTileLoaded(tile) && !IsTileLoading(tile))
				{
					LoadTile(vec2i(x, y));
				}
			}
		}

		auto loadingTilesIt = myTilesToLoad.begin();
		while (loadingTilesIt < myTilesToLoad.end())
		{
			if ((*loadingTilesIt)->IsBuilt())
			{
				myActiveTiles.push_back(*loadingTilesIt);
				SceneManager::GetInstance()->GetModelManager()->AddTerrainTile(*loadingTilesIt, myTileSize, myResolution);
				//std::cout << "adding a tile " << myActiveTiles.size() << std::endl;
				loadingTilesIt = myTilesToLoad.erase(loadingTilesIt);
			}
			else
			{
				++loadingTilesIt;
			}
		}

		auto activeTilesIt = myActiveTiles.begin();
		while (activeTilesIt < myActiveTiles.end())
		{
			const vec2i tileIndex = (*activeTilesIt)->GetGridIndex();
			if (pow(tileIndex.x - positionOnGrid.x, 2) + pow(tileIndex.y - positionOnGrid.y, 2) > pow(myCachedRadius, 2))
			{
				SceneManager::GetInstance()->GetModelManager()->RemoveTerrainTile(tileIndex);
				delete *activeTilesIt;
				activeTilesIt = myActiveTiles.erase(activeTilesIt);
			}
			else
			{
				++activeTilesIt;
			}
		}
	}

	bool TerrainManager::IsTileLoaded(const vec2i& aTileIndex)
	{
		return std::find_if(myActiveTiles.begin(), myActiveTiles.end(),
			[aTileIndex](TerrainTile* aLoadedTile)
		{return aLoadedTile->GetGridIndex() == aTileIndex; })
			!= myActiveTiles.end();
	}

	bool TerrainManager::IsTileLoading(const vec2i& aTileIndex)
	{
		return std::find_if(myTilesToLoad.begin(), myTilesToLoad.end(),
			[aTileIndex](TerrainTile* aTileToLoad) 
		{return aTileToLoad->GetGridIndex() == aTileIndex; })
			!= myTilesToLoad.end();
	}

	void TerrainManager::LoadTile(const vec2i& aGridIndex)
	{
		auto tilePosition = vec2f((float) aGridIndex.x * (float) myTileSize * myResolution, (float) aGridIndex.y * (float) myTileSize * myResolution);
		const auto cell = SampleGrid(tilePosition);

		auto tile = new TerrainTile(aGridIndex, myTileSize, myResolution, cell);
		myTilesToLoad.push_back(tile);
		myTileBuilder->BuildTileRequest(tile);
	}
}