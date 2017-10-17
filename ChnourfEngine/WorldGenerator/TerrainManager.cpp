#pragma once
#include <algorithm>
#include "../Managers/SceneManager.h"
#include "../Managers/ModelManager.h"
#include "TerrainManager.h"
#include "TerrainCell.h"

#include "TerrainGenerationFunctions.h"

#include <array>
#include <iostream>
#include <fstream>
#include <sstream>

namespace Manager
{
	TerrainManager::TerrainManager() :
		myCellSize(0),
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
			std::array<float, 4> landscapeRepartitionPercentages;
			unsigned int currentPercentage = 0u;

			while (ss >> field >> value)
			{
				if (strcmp(field.c_str(), "DetectionRadius") == 0)
				{
					myDetectionRadius = (unsigned int) value;
					myCachedRadius = (unsigned int) (value + 1);
				}
				else if (strcmp(field.c_str(), "CellSize") == 0)
				{
					myCellSize = (unsigned int) value;
				}
				else if (strcmp(field.c_str(), "CellResolution") == 0)
				{
					myResolution = value;
				}
				else if (strcmp(field.c_str(), "LandscapeRepartitionPercentages") == 0)
				{
					landscapeRepartitionPercentages[currentPercentage] = value;
					if (currentPercentage >= landscapeRepartitionPercentages.size())
					{
						assert(false);
					}
					++currentPercentage;
				}
			}

			TerrainGeneration::SetLandscapeRepartitionConstants(landscapeRepartitionPercentages);
		}

		file.close();

		auto mustBeAPowerOf2 = myCellSize - 1;
		assert(((mustBeAPowerOf2 != 0) && ((mustBeAPowerOf2 & (~mustBeAPowerOf2 + 1)) == mustBeAPowerOf2)));
		myCellBuilder = std::make_unique<TerrainCellBuilder> (myCellSize, myResolution);
	}

	TerrainManager::~TerrainManager()
	{
		for (auto cell : myActiveCells)
		{
			delete cell;
		}
	}

	void TerrainManager::Update(const vec3f& aPlayerPosition)
	{
		myCellBuilder->Update();

		const vec2i positionOnGrid = vec2i(aPlayerPosition.x / (myCellSize*myResolution), aPlayerPosition.z / (myCellSize*myResolution));

		// detection square
		const int minX = positionOnGrid.x - myDetectionRadius;
		const int maxX = positionOnGrid.x + myDetectionRadius;
		const int minY = positionOnGrid.y - myDetectionRadius;
		const int maxY = positionOnGrid.y + myDetectionRadius;
		for (int x = minX; x < maxX; x++)
		{
			for (int y = minY; y < maxY; y++)
			{
				auto cell = vec2i(x, y);

				// detection circle
				if (pow(x - positionOnGrid.x, 2) + pow(y - positionOnGrid.y, 2) > pow(myDetectionRadius, 2))
				{
					continue;
				}
				
				if (!IsCellLoaded(cell) && !IsCellLoading(cell))
				{
					LoadCell(vec2i(x, y));
				}
			}
		}

		auto loadingCellsIt = myCellsToLoad.begin();
		while (loadingCellsIt < myCellsToLoad.end())
		{
			if ((*loadingCellsIt)->IsBuilt())
			{
				myActiveCells.push_back(*loadingCellsIt);
				SceneManager::GetInstance()->GetModelManager()->AddTerrainCell(*loadingCellsIt, myCellSize, myResolution);
				std::cout << "adding a cell " << myActiveCells.size() << std::endl;
				loadingCellsIt = myCellsToLoad.erase(loadingCellsIt);
			}
			else
			{
				++loadingCellsIt;
			}
		}

		auto activeCellsIt = myActiveCells.begin();
		while (activeCellsIt < myActiveCells.end())
		{
			const vec2i tileIndex = (*activeCellsIt)->GetGridIndex();
			if (pow(tileIndex.x - positionOnGrid.x, 2) + pow(tileIndex.y - positionOnGrid.y, 2) > pow(myCachedRadius, 2))
			{
				SceneManager::GetInstance()->GetModelManager()->RemoveTerrainCell(tileIndex);
				delete *activeCellsIt;
				activeCellsIt = myActiveCells.erase(activeCellsIt);
			}
			else
			{
				++activeCellsIt;
			}
		}
	}

	bool TerrainManager::IsCellLoaded(const vec2i& aCellIndex)
	{
		auto result = false;
		auto it = std::find_if(myActiveCells.begin(), myActiveCells.end(), [aCellIndex](TerrainCell* aLoadedCell) {return aLoadedCell->GetGridIndex() == aCellIndex; });
		if (it != myActiveCells.end())
		{
			result = true;
		}

		return result;
	}

	bool TerrainManager::IsCellLoading(const vec2i& aCellIndex)
	{
		auto result = false;
		auto it = std::find_if(myCellsToLoad.begin(), myCellsToLoad.end(), [aCellIndex](TerrainCell* aCellToLoad) {return aCellToLoad->GetGridIndex() == aCellIndex; });
		if (it != myCellsToLoad.end())
		{
			result = true;
		}

		return result;
	}

	void TerrainManager::LoadCell(const vec2i& aGridIndex)
	{
		auto cell = new TerrainCell(aGridIndex, myCellSize, myResolution);
		myCellsToLoad.push_back(cell);
		myCellBuilder->BuildCellRequest(cell);
	}
}