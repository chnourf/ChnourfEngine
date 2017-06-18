#pragma once
#include <vector>
#include "glm\glm.hpp"
#include "../Core/Vector.h"

typedef unsigned short ushort;

struct TerrainElement
{
	TerrainElement(float anElevation, const glm::vec3 aNormal):
		myElevation(anElevation),
		myNormal(aNormal)
	{}

	float myElevation;
	//maybe store as short
	glm::vec3 myNormal;
};

class TerrainCell
{
public:
	TerrainCell(const vec2i& aGridIndex, int aCellSize, float aResolution);
	inline const vec2i& GetGridIndex() const { return myGridIndex; }
	__forceinline void AddTerrainElement(TerrainElement anElement) { myElements.push_back(anElement); }
	__forceinline TerrainElement GetElement(const unsigned int anIndex) const { return *(myElements.begin() + anIndex); }
	__forceinline bool IsBuilt() { return myIsBuilt; }
	__forceinline void AddGrassPosition(vec3f aPos) { myGrassSpots.push_back(aPos); }
	void OnFinishBuild();

	std::vector<vec3f> GetGrassSpots() const { return myGrassSpots; }

	__forceinline float GetMinHeight() const { return myMinHeight; }
	__forceinline float GetMaxHeight() const { return myMaxHeight; }

	__forceinline void SetMinHeight(float aHeight) { myMinHeight = aHeight; }
	__forceinline void SetMaxHeight(float aHeight)  { myMaxHeight = aHeight; }

	vec3f VerticalRaycast(const vec2f& aPositionInTheCell); // if you use a global position, convert it first !

private:
	vec2i myGridIndex;
	bool myIsBuilt;
	std::vector<TerrainElement> myElements;
	std::vector<vec3f> myGrassSpots;
	int myMoisture;
	int myTemperature;
	int myBiome;

	unsigned int myCellSize;
	float myResolution;

	float myMinHeight;
	float myMaxHeight;

	int myCurrentLOD;
};
