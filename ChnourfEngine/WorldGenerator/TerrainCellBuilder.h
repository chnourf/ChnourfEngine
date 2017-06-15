#pragma once

#include "glm\glm.hpp"
#include "../Core/PerlinNoise.h"

#include <queue>
#include <vector>
#include <future>

class TerrainCell;
struct TerrainElement;

struct ErosionParams
{
	float Kq; // soil carry capacity
	float Kevap; // evaporation speed
	float Kerosion; // erosion speed
	float Kdepos; // deposition speed
	float Ki; // inertia
	float minSlope;
	float g; // gravity
};

struct TerrainCellBuildingTask
{
	TerrainCellBuildingTask(const int aSeed, const unsigned int aCellSize, float aCellResolution, TerrainCell* anEmptyCell);
	~TerrainCellBuildingTask();

	PerlinNoise myPerlin;
	int myNoiseDepth;
	unsigned int mySeed;

	unsigned int myCellSize;
	float myCellResolution;

	std::future<void> myHandle;

	void BuildCell(TerrainCell* aCell);
	float SamplePerlinNoise(float aX, float aY);
	void ComputeErosion(std::vector<TerrainElement>& elevationMap, const unsigned int iterations, const ErosionParams& params);
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
