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

	const unsigned int myTileSize;
	const float myTileResolution;

	std::future<void> myHandle;

	void BuildTile(TerrainTile* aTile);
};

class TerrainTileBuilder
{
public:
	TerrainTileBuilder(const int aTileSize,const float aResolution, unsigned int aSeed);

	void BuildTileRequest(TerrainTile* aTile);
	void CancelBuildRequest(const TerrainTile* aTile);

	void Update();

private:
	std::vector<TerrainTileBuildingTask*> myLoadingTasks;
	const int myMaximumThreadLoad = 4;
	std::deque<TerrainTile*> myLoadingQueue;

	unsigned int myTileSize;
	unsigned int mySeed;
	float myTileResolution;
};
