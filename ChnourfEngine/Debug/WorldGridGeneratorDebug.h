#pragma once
#include "../WorldGenerator/Geometry.h"
#include "../WorldGenerator/WorldGrid.h"

namespace MapDebug
{
	void DrawAndSaveDebugImages(const TerrainGeneration::WorldGrid& aGrid);

	void CreateMinimapTexture();
	void SetFacingDirection(const vec2f& aFacingDirection);
	void RenderMinimap(const vec2f& aPosition);
}

