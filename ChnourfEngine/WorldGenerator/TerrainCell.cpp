#include "TerrainCell.h"


TerrainCell::TerrainCell(const vec2i& aGridIndex, int aCellSize):
	myGridIndex(aGridIndex)
{
	myElements.reserve(aCellSize*aCellSize);
}
