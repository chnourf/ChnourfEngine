#include "TerrainCell.h"


TerrainCell::TerrainCell(const vec2i& aGridIndex, int aCellSize, float aResolution) :
	myGridIndex(aGridIndex),
	myIsBuilt(false),
	myCellSize(aCellSize),
	myResolution(aResolution)
{
	myElements.reserve((aCellSize)*(aCellSize));
}

vec3f TerrainCell::VerticalRaycast(const vec2f& aPosition)
{
	// we suppose cell is computed

	const int x = floor((aPosition.x - myCellSize * myGridIndex.x) * myResolution);
	const int y = floor((aPosition.y - myCellSize * myGridIndex.y) * myResolution);

	const float h = myElements[x + myCellSize * y].myElevation;
	const float h01 = myElements[x + myCellSize * (y + 1)].myElevation;
	const float h10 = myElements[x + 1 + myCellSize * y].myElevation;
	const float h11 = myElements[x + 1 + myCellSize * (y + 1)].myElevation;

	bool isEven = x % 2 == 0; // because TerrainCellModel alternates the way the faces are displayed

	return vec3f(0.f);
}

void TerrainCell::OnFinishBuild()
{
	myIsBuilt = true;
}
