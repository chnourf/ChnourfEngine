#pragma once
#include <algorithm>
#include "TerrainManager.h"
#include "TerrainCell.h"

namespace Manager
{
	TerrainManager::TerrainManager()
	{
		myCellBuilder = std::make_unique<TerrainCellBuilder> (myCellSize);
	}

	TerrainManager::~TerrainManager()
	{
		for (auto cell : myActivesCells)
		{
			delete cell;
		}
	}

	void TerrainManager::Update(const vec3f& aPlayerPosition)
	{
		//careful with the position sampled in CellBuilder, are they the same ? Where is (0,0) ?
		const vec2i positionOnGrid = vec2i(aPlayerPosition.x / (myCellSize*myResolution), aPlayerPosition.y / (myCellSize*myResolution));

		auto it = std::find_if(myActivesCells.begin(), myActivesCells.end(), [](TerrainCell* aCell) {return aCell->GetGridIndex() == vec2i(0, 0); });
		if (it == myActivesCells.end())
		{
			LoadCell(vec2i(0, 0));
		}
	}

	void TerrainManager::LoadCell(const vec2i& aGridIndex)
	{
		// check that the cell is not already loading when we create another
		auto cell = new TerrainCell(aGridIndex, myCellSize);
		myCellBuilder->BuildCell(cell);
		myActivesCells.push_back(cell);
	}
}