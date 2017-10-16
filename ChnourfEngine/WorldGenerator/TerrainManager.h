#pragma once
#include <memory>
#include <vector>
#include <glm\glm.hpp>
#include "TerrainCellBuilder.h"

#include "../Core/Singleton.h"
#include "../Core/Vector.h"

class TerrainCell;

enum class Biome
{
	Tundra,
	BorealForest,
	Woodland,
	Grassland,
	SeasonalForest,
	RainForest,
	Forest,
	Swamp,
	Desert,
	Savanna,
	Count,
};

enum class Reliefs
{
	Sea,
	Plain,
	Hills,
	Mountains,
	HighMountains,
	Count,
};

//inline Biome GetBiome(const float aTemperature, const float aRainfall)
//{
//	if (aRainfall > 0.75f)
//	{
//		if (aTemperature > 0.75f)
//		{
//			return RainForest;
//		}
//		return Swamp;
//	}
//	else if (aRainfall > 0.5f)
//	{
//		if (aTemperature > 0.75f)
//		{
//			return SeasonalForest;
//		}
//		if (aTemperature > 0.5f)
//		{
//			return Forest;
//		}
//		return BorealForest;
//	}
//	else if (aRainfall > 0.25f)
//	{
//
//	}
//}

namespace Manager
{
	class TerrainManager : public Singleton<TerrainManager>
	{
	public:
		TerrainManager();
		~TerrainManager();

		void Update(const vec3f& aPlayerPosition);

		inline unsigned int GetCellSize() { return myCellSize; }
		inline float GetResolution() { return myResolution; }

	private:
		std::vector<TerrainCell*> myActiveCells;
		std::vector<TerrainCell*> myCachedCells; // to prevent when the character moves back and forth at the border of a cell
		std::vector<TerrainCell*> myCellsToLoad;
		std::vector<vec2i> myCellsToRemove;

		std::unique_ptr<TerrainCellBuilder> myCellBuilder;

		void LoadCell(const vec2i& aGridIndex);

		bool IsCellLoaded(const vec2i& aCellIndex);
		bool IsCellLoading(const vec2i& aCellIndex);

		unsigned int myCellSize;
		float myResolution;
		unsigned int myDetectionRadius;
		unsigned int myCachedRadius;
	};
}
