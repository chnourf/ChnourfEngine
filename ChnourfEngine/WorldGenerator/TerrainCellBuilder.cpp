#include "TerrainCellBuilder.h"
#include"TerrainCell.h"
#include <time.h>
#include <iostream>

TerrainCellBuilder::TerrainCellBuilder(int aCellSize):
	myCellSize(aCellSize)
{
	unsigned int seed = time(NULL);
	srand(seed);
	myPerlin = PerlinNoise(seed);
}

void TerrainCellBuilder::BuildCell(TerrainCell* aCell)
{
	if (!aCell)
	{
		std::cout << "ERROR : cell given to build does not exist" << std::endl;
		return;
	}

	for (unsigned int i = 0; i < myCellSize; ++i) {     // y
		for (unsigned int j = 0; j < myCellSize; ++j) {  // x
			const float x = (float)j / ((float)myCellSize) + aCell->GetGridIndex().x;
			const float y = (float)i / ((float)myCellSize) + aCell->GetGridIndex().y;

			auto cellNoise = 0.f;

			// Typical Perlin noise
			for (int d = 1; d <= myNoiseDepth; d++)
			{
				float factor = pow(2, d);
				cellNoise += 1 / factor * myPerlin.noise(factor*x, factor*y, 0);
			}

			ushort elevation = (ushort) (cellNoise * 0xffff);
			aCell->AddTerrainElement(TerrainElement(elevation));
		}
	}
}
