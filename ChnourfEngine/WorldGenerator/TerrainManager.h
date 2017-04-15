#pragma once
#include <memory>
#include <vector>
#include <glm\glm.hpp>
#include "TerrainCellBuilder.h"

#include "../Core/Vector.h"

class TerrainCell;


namespace Manager
{
	class TerrainManager
	{
	public:
		TerrainManager();
		~TerrainManager();

		void Update(const vec3f& aPlayerPosition);

	private:
		std::vector<TerrainCell*> myActivesCells;
		std::vector<TerrainCell*> myCachedCells; // to prevent when the character moves back and forth at the border of a cell
		std::vector<vec2i> myCellsToLoad;
		std::vector<vec2i> myCellsToRemove;

		std::unique_ptr<TerrainCellBuilder> myCellBuilder; // should be a pointer to a "builder manager" that dispatches works on several cellbuilder

		void LoadCell(const vec2i& aGridIndex);
		void UnloadCell(const vec2i& aGridIndex);

		const int myCellSize = 256; //256x256 square
		const float myResolution = 0.5f; // 50 cm between each element
	};
}
