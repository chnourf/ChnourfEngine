#pragma once

#include "glm\glm.hpp"
#include "../Core/PerlinNoise.h"

class TerrainCell;

class TerrainCellBuilder
{
public:
	TerrainCellBuilder(int aCellSize);

	void BuildCell(TerrainCell* aCell);

private:
	PerlinNoise myPerlin;
	int myNoiseDepth = 12;
	int myCellSize;
};
