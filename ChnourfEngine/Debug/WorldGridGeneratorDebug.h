#pragma once
#include "../WorldGenerator/Geometry.h"
#include "../WorldGenerator/WorldGrid.h"

namespace Debug
{
	vec3f DeduceBiomeColor(TerrainGeneration::Biome aBiome);
	void DrawGrid(const TerrainGeneration::WorldGrid& aGrid);
}

