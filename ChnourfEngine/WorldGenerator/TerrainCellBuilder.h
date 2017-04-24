#pragma once

#include "glm\glm.hpp"
#include "../Core/PerlinNoise.h"

#include <queue>
#include <vector>
#include <future>

class TerrainCell;

struct TerrainCellBuildingTask
{
	TerrainCellBuildingTask(const int aSeed, const unsigned int aCellSize, float aCellResolution, TerrainCell* anEmptyCell);
	~TerrainCellBuildingTask();

	PerlinNoise myPerlin;
	int myNoiseDepth;

	unsigned int myCellSize;
	float myCellResolution;

	std::future<void> myHandle;

	void BuildCell(TerrainCell* aCell);
	float SamplePerlinNoise(const float aX, const float aY);
};

class TerrainCellBuilder
{
public:
	TerrainCellBuilder(const int aCellSize,const float aResolution);

	void BuildCellRequest(TerrainCell* aCell);

	void Update();

private:
	std::vector<TerrainCellBuildingTask*> myLoadingTasks;
	unsigned int mySeed;
	const int myMaximumThreadLoad = 8;
	std::queue<TerrainCell*> myLoadingQueue;

	unsigned int myCellSize;
	float myCellResolution;
};
