#pragma once
#include <memory>
#include <vector>
#include <glm\glm.hpp>
#include "TerrainTileBuilder.h"

#include "../Core/Singleton.h"
#include "../Core/Vector.h"
#include "WorldGrid.h"

class TerrainTile;
namespace TerrainGeneration
{
	struct Cell;
}

namespace Manager
{
	class TerrainManager : public Singleton<TerrainManager>
	{
	public:
		TerrainManager();
		~TerrainManager();

		void Update(const vec3f& aPlayerPosition);

		inline unsigned int GetTileSize() { return myTileSize; }
		inline float GetResolution() { return myResolution; }

		inline const TerrainGeneration::Cell* SampleGrid(const vec2f& aPosition) { return myWorldGrid->SampleGridCell(aPosition); }

	private:
		std::vector<TerrainTile*> myActiveTiles;
		std::vector<TerrainTile*> myTilesToLoad;
		std::vector<vec2i> myTilesToRemove;

		std::unique_ptr<TerrainTileBuilder> myTileBuilder;
		std::unique_ptr<TerrainGeneration::WorldGrid> myWorldGrid;

		void LoadTile(const vec2i& aGridIndex);

		bool IsTileLoaded(const vec2i& aTileIndex);
		bool IsTileLoading(const vec2i& aTileIndex);

		unsigned int myTileSize;
		float myResolution;
		unsigned int myDetectionRadius;
		unsigned int myCachedRadius;

		unsigned int mySeed;
	};
}
