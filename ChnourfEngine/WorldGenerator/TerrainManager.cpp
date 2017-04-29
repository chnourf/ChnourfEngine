#pragma once
#include <algorithm>
#include "../Managers/SceneManager.h"
#include "../Managers/ModelManager.h"
#include "TerrainManager.h"
#include "TerrainCell.h"

#include <iostream>

namespace Manager
{
	TerrainManager::TerrainManager():
		myDetectionRadius(5)
	{
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

		//careful with the position sampled in CellBuilder, are they the same ? Where is (0,0) ?
		const vec2i positionOnGrid = vec2i(aPlayerPosition.x / (myCellSize*myResolution), aPlayerPosition.z / (myCellSize*myResolution));

		for (int x = positionOnGrid.x - myDetectionRadius; x < positionOnGrid.x + myDetectionRadius; x++)
		{
			for (int y = positionOnGrid.y - myDetectionRadius; y < positionOnGrid.y + myDetectionRadius; y++)
			{
				auto cell = vec2i(x, y);
				if (!IsCellLoaded(cell) && !IsCellLoading(cell))
				{
					LoadCell(vec2i(x, y));
				}
			}
		}

		auto it = myCellsToLoad.begin();
		while (it < myCellsToLoad.end())
		{
			if ((*it)->IsBuilt())
			{
				myActiveCells.push_back(*it);
				SceneManager::GetInstance()->GetModelManager()->AddTerrainCell(*it, myCellSize, myResolution);
				std::cout << "adding a cell " << myActiveCells.size() << std::endl;
				it = myCellsToLoad.erase(it);
			}
			else
			{
				++it;
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
		auto cell = new TerrainCell(aGridIndex, myCellSize);
		myCellsToLoad.push_back(cell);
		myCellBuilder->BuildCellRequest(cell);
	}
}