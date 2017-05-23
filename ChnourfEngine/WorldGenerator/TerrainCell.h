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
	TerrainCell(const vec2i& aGridIndex, int aCellSize);
	inline const vec2i& GetGridIndex() const { return myGridIndex; }
	__forceinline void AddTerrainElement(TerrainElement anElement) { myElements.push_back(anElement); }
	__forceinline TerrainElement GetElement(const unsigned int anIndex) const { return *(myElements.begin() + anIndex); }
	__forceinline bool IsBuilt() { return myIsBuilt; }
	void OnFinishBuild();

private:
	vec2i myGridIndex;
	bool myIsBuilt;
	std::vector<TerrainElement> myElements;
	int myMoisture;
	int myTemperature;
	int myBiome;

	int myCurrentLOD;
};
