#pragma once

#include "glm\glm.hpp"
#include "../Core/PerlinNoise.h"

#include <queue>
#include <random>
#include <vector>
#include <future>

class TerrainCell;
struct TerrainElement;

namespace TerrainGeneration
{
	struct ErosionParams;
}

struct TerrainCellBuildingTask
{
	TerrainCellBuildingTask(const int aSeed, const unsigned int aCellSize, float aCellResolution, TerrainCell* anEmptyCell);
	~TerrainCellBuildingTask();

	PerlinNoise myPerlin;
	int myNoiseDepth;
	std::default_random_engine myRandomEngine;

	unsigned int myCellSize;
	float myCellResolution;

	std::future<void> myHandle;

	void BuildCell(TerrainCell* aCell);
	float SamplePerlinNoise(float aX, float aY);
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
	const int myMaximumThreadLoad = 4;
	std::queue<TerrainCell*> myLoadingQueue;

	unsigned int myCellSize;
	float myCellResolution;
};
