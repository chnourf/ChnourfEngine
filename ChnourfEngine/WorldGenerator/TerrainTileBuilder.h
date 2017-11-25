#pragma once

#include "glm\glm.hpp"
#include "../Core/PerlinNoise.h"

#include <queue>
#include <random>
#include <vector>
#include <future>

class TerrainTile;
struct TerrainElement;

namespace TerrainGeneration
{
	struct ErosionParams;
	struct Cell;
}

struct TerrainTileBuildingTask
{
	TerrainTileBuildingTask(const int aSeed, const unsigned int aTileSize, float aTileResolution, TerrainTile* anEmptyTile);
	~TerrainTileBuildingTask();

	PerlinNoise myPerlin;
	int myNoiseDepth;
	std::default_random_engine myRandomEngine;

	unsigned int myTileSize;
	float myTileResolution;

	std::future<void> myHandle;

	void BuildTile(TerrainTile* aTile);
	float SamplePerlinNoise(float aX, float aY);

	const TerrainGeneration::Cell* myWorldCell;
};

class TerrainTileBuilder
{
public:
	TerrainTileBuilder(const int aTileSize,const float aResolution, unsigned int aSeed);

	void BuildTileRequest(TerrainTile* aTile);

	void Update();

private:
	std::vector<TerrainTileBuildingTask*> myLoadingTasks;
	const int myMaximumThreadLoad = 4;
	std::queue<TerrainTile*> myLoadingQueue;

	unsigned int myTileSize;
	unsigned int mySeed;
	float myTileResolution;
};
