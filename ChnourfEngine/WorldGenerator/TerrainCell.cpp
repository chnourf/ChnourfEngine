#include "TerrainCell.h"


TerrainCell::TerrainCell(const vec2i& aGridIndex, int aCellSize):
	myGridIndex(aGridIndex),
	myIsBuilt(false)
{
	myElements.reserve(aCellSize*aCellSize);
}

void TerrainCell::OnFinishBuild()
{
	myIsBuilt = true;
}
