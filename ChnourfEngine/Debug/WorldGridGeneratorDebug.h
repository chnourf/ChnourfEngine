#pragma once
#include "../WorldGenerator/Geometry.h"

namespace Debug
{
	vec3f DeduceBiomeColor(TerrainGeneration::Biome aBiome);
	void DrawGrid(const TerrainGeneration::Grid& aGrid, const unsigned int anImageSize, const unsigned int metersPerPixel);
}

