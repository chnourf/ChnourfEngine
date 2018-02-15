#include "TerrainTile.h"


TerrainTile::TerrainTile(const vec2i& aGridIndex, int aTileSize, float aResolution) :
	myGridIndex(aGridIndex),
	myIsBuilt(false),
	myHasBuildStarted(false),
	myTileSize(aTileSize),
	myResolution(aResolution),
	myTotalBuildTime(0.f),
	myErosionBuildTime(0.f)
{
	myElements.reserve((aTileSize)*(aTileSize));
}

float TerrainTile::GetY(float aX, float aZ) const
{
	float  u = (aX - myGridIndex.x * (int)myTileSize * myResolution);
	float  v = (aZ - myGridIndex.y * (int)myTileSize * myResolution);

	float  fx = u / myResolution;
	int          ix0 = glm::clamp(int(fx), 0, (int)myTileSize - 1);

	float  fz = v / myResolution;
	int          iz0 = glm::clamp(int(fz), 0, (int)myTileSize - 1);

	int ix1 = glm::min(ix0 + 1, (int) myTileSize - 1);
	int iz1 = glm::min(iz0 + 1, (int) myTileSize - 1);
	
	float weightEX = fx - ix0;
	float weightSX = 1.0f - weightEX;
	float weightEZ = fz - iz0;
	float weightSZ = 1.0f - weightEZ;
	
	float factor0 = weightSX * weightSZ;
	float factor1 = weightSX * weightEZ;
	float factor2 = weightEX * weightEZ;
	float factor3 = weightEX * weightSZ;
	
	
	float height0 = myElements[ix0 * myTileSize + iz0].myElevation; // (0,0)
	float height1 = myElements[ix1 * myTileSize + iz0].myElevation; // (0,1)
	float height2 = myElements[ix1 * myTileSize + iz1].myElevation; // (1,1)
	float height3 = myElements[ix0 * myTileSize + iz1].myElevation; // (1,0)
	
	return (height0 * factor0 +
		height1 * factor1 +
		height2 * factor2 +
		height3 * factor3);
}

vec3f TerrainTile::GetNormal(float aX, float aZ) const
{
	float  u = (aX - myGridIndex.x * (int)myTileSize);
	float  v = (aZ - myGridIndex.y * (int)myTileSize);
	float  fx = u * myResolution * (myTileSize - 1);
	int          ix0 = glm::clamp(int(fx), 0, (int)myTileSize - 1);

	float  fz = v * myResolution * (myTileSize - 1);
	int          iz0 = glm::clamp(int(fz), 0, (int)myTileSize - 1);

	int ix1 = glm::min(ix0 + 1, (int)myTileSize - 1);
	int iz1 = glm::min(iz0 + 1, (int)myTileSize - 1);

	float weightEX = fx - ix0;
	float weightSX = 1.0f - weightEX;
	float weightEZ = fz - iz0;
	float weightSZ = 1.0f - weightEZ;

	float factor0 = weightSX * weightSZ;
	float factor1 = weightSX * weightEZ;
	float factor2 = weightEX * weightEZ;
	float factor3 = weightEX * weightSZ;


	auto height0 = myElements[iz0 * myTileSize + ix0].myNormal; // (0,0)
	auto height1 = myElements[iz1 * myTileSize + ix0].myNormal; // (0,1)
	auto height2 = myElements[iz1 * myTileSize + ix1].myNormal; // (1,1)
	auto height3 = myElements[iz0 * myTileSize + ix1].myNormal; // (1,0)

	return (height0 * factor0 +
		height1 * factor1 +
		height2 * factor2 +
		height3 * factor3);
}

void TerrainTile::OnStartBuild()
{
	myHasBuildStarted = true;
}


void TerrainTile::OnFinishBuild()
{
	myIsBuilt = true;
}
