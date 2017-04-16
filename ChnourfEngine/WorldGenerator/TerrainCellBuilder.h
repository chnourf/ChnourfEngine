#pragma once

#include "glm\glm.hpp"
#include "../Core/PerlinNoise.h"

#include <vector>
#include <future>

class TerrainCell;

struct TerrainCellBuildingTask
{
	TerrainCellBuildingTask(const int aSeed, const unsigned int aCellSize, TerrainCell* aCell);
	~TerrainCellBuildingTask();

	PerlinNoise myPerlin;
	int myNoiseDepth = 6;
	unsigned int myCellSize;
	std::future<void> myHandle;

	void BuildCell(TerrainCell* aCell);
};

class TerrainCellBuilder
{
public:
	TerrainCellBuilder(int aCellSize);

	void BuildCellRequest(TerrainCell* aCell);

	void Update();

private:
	std::vector<TerrainCellBuildingTask*> myLoadingTasks;
	int mySeed;
	const int myMaximumThreadLoad = 4;
	int myCurrentThreadLoad;
	std::vector<TerrainCell*> myLoadingQueue;
	unsigned int myCellSize;
};
