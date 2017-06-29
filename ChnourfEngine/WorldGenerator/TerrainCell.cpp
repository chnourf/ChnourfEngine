#include "TerrainCell.h"


TerrainCell::TerrainCell(const vec2i& aGridIndex, int aCellSize, float aResolution) :
	myGridIndex(aGridIndex),
	myIsBuilt(false),
	myCellSize(aCellSize),
	myResolution(aResolution)
{
	myElements.reserve((aCellSize)*(aCellSize));
}

float TerrainCell::GetY(float aX, float aZ) const
{
	float  u = (aX - myGridIndex.x * (int)myCellSize * myResolution);
	float  v = (aZ - myGridIndex.y * (int)myCellSize * myResolution);

	float  fx = u / myResolution;
	int          ix0 = glm::clamp(int(fx), 0, (int)myCellSize - 1);

	float  fz = v / myResolution;
	int          iz0 = glm::clamp(int(fz), 0, (int)myCellSize - 1);

	int ix1 = glm::min(ix0 + 1, (int) myCellSize - 1);
	int iz1 = glm::min(iz0 + 1, (int) myCellSize - 1);
	
	float weightEX = fx - ix0;
	float weightSX = 1.0f - weightEX;
	float weightEZ = fz - iz0;
	float weightSZ = 1.0f - weightEZ;
	
	float factor0 = weightSX * weightSZ;
	float factor1 = weightSX * weightEZ;
	float factor2 = weightEX * weightEZ;
	float factor3 = weightEX * weightSZ;
	
	
	float height0 = myElements[ix0 * myCellSize + iz0].myElevation; // (0,0)
	float height1 = myElements[ix1 * myCellSize + iz0].myElevation; // (0,1)
	float height2 = myElements[ix1 * myCellSize + iz1].myElevation; // (1,1)
	float height3 = myElements[ix0 * myCellSize + iz1].myElevation; // (1,0)
	
	return (height0 * factor0 +
		height1 * factor1 +
		height2 * factor2 +
		height3 * factor3);
}

vec3f TerrainCell::GetNormal(float aX, float aZ) const
{
	float  u = (aX - myGridIndex.x * (int)myCellSize);
	float  v = (aZ - myGridIndex.y * (int)myCellSize);
	float  fx = u * myResolution * (myCellSize - 1);
	int          ix0 = glm::clamp(int(fx), 0, (int)myCellSize - 1);

	float  fz = v * myResolution * (myCellSize - 1);
	int          iz0 = glm::clamp(int(fz), 0, (int)myCellSize - 1);

	int ix1 = glm::min(ix0 + 1, (int)myCellSize - 1);
	int iz1 = glm::min(iz0 + 1, (int)myCellSize - 1);

	float weightEX = fx - ix0;
	float weightSX = 1.0f - weightEX;
	float weightEZ = fz - iz0;
	float weightSZ = 1.0f - weightEZ;

	float factor0 = weightSX * weightSZ;
	float factor1 = weightSX * weightEZ;
	float factor2 = weightEX * weightEZ;
	float factor3 = weightEX * weightSZ;


	auto height0 = myElements[iz0 * myCellSize + ix0].myNormal; // (0,0)
	auto height1 = myElements[iz1 * myCellSize + ix0].myNormal; // (0,1)
	auto height2 = myElements[iz1 * myCellSize + ix1].myNormal; // (1,1)
	auto height3 = myElements[iz0 * myCellSize + ix1].myNormal; // (1,0)

	return (height0 * factor0 +
		height1 * factor1 +
		height2 * factor2 +
		height3 * factor3);
}

void TerrainCell::OnFinishBuild()
{
	myIsBuilt = true;
}
