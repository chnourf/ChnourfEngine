#pragma once
#include <memory>
#include <vector>
#include <glm\glm.hpp>
#include "TerrainCellBuilder.h"

#include "../Core/Singleton.h"
#include "../Core/Vector.h"

class TerrainCell;

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
