#pragma once
#include <vector>
//#include <iostream>
//#include "../../Dependencies/glew/glew.h"
//#include "../../Dependencies/freeglut/freeglut.h"
//#include "../VertexFormat.h"
//#include "../TextureFormat.h"
#include "glm\glm.hpp"
#include "../Core/Vector.h"

typedef unsigned short ushort;

struct TerrainElement
{
	TerrainElement(ushort anElevation):
		myElevation(anElevation)
	{}

	ushort myElevation;
};

class TerrainCell
{
public:
	TerrainCell(const vec2i& aGridIndex, int aCellSize);
	inline const vec2i& GetGridIndex() { return myGridIndex; }
	__forceinline void AddTerrainElement(TerrainElement anElement) { myElements.push_back(anElement); }
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
