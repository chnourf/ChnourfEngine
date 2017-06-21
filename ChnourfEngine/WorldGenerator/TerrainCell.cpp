#include "TerrainCell.h"


TerrainCell::TerrainCell(const vec2i& aGridIndex, int aCellSize, float aResolution) :
	myGridIndex(aGridIndex),
	myIsBuilt(false),
	myCellSize(aCellSize),
	myResolution(aResolution)
{
	myElements.reserve((aCellSize)*(aCellSize));
	myGrassSpots.reserve((aCellSize) * (aCellSize)); // to be changed
}

vec3f TerrainCell::VerticalRaycast(const vec2f& aPositionInTheCell)
{
	vec3f res = vec3f(0.f);
	
	double dummy;
	const float relX = modf(aPositionInTheCell.x, &dummy);
	const float relY = modf(aPositionInTheCell.y, &dummy);

	const int x = floor(aPositionInTheCell.x);
	const int y = floor(aPositionInTheCell.y);

	// we suppose cell is computed
	const float h = myElements[x + myCellSize * y].myElevation;
	const float h01 = myElements[x + myCellSize * (y + 1)].myElevation;
	const float h10 = myElements[x + 1 + myCellSize * y].myElevation;
	const float h11 = myElements[x + 1 + myCellSize * (y + 1)].myElevation;

	bool isEven = x % 2 == 0; // because TerrainCellModel alternates the way the faces are displayed

	if (isEven)
	{
		if (relY + 1.f - relX < 0.f)
		{
			res = vec3f(aPositionInTheCell.x, relX * (h10 - h) + relY * (h01 - h) + h, aPositionInTheCell.y);
		}
		else
		{
			res = vec3f(aPositionInTheCell.x, (1.f - relX) * (h01 - h11) + (1.f - relY) * (h10 - h11) + h11 , aPositionInTheCell.y);
		}
	}
	else
	{
		if (relY + relX > 0.f)
		{
			res = vec3f(aPositionInTheCell.x, relX * (h11 - h01) + (1- relY) * (h - h01) + h01, aPositionInTheCell.y);
		}
		else
		{
			res = vec3f(aPositionInTheCell.x, (1 - relX) * (h - h10) + relY * (h11 - h10) + h10, aPositionInTheCell.y);
		}
	}

	return res;
}

void TerrainCell::OnFinishBuild()
{
	myIsBuilt = true;
}
