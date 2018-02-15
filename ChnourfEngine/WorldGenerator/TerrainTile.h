#pragma once
#include <vector>
#include "glm\glm.hpp"
#include "../Core/Vector.h"

#include <iostream>

typedef unsigned short ushort;

namespace TerrainGeneration
{
	struct Cell;
}

struct TerrainElement
{
	TerrainElement(float anElevation, const glm::vec3 aNormal, const unsigned char aRainfall, const unsigned char aTemperature, const unsigned char anErodedCoeff):
		myElevation(anElevation),
		myNormal(aNormal),
		myRainfall(aRainfall),
		myTemperature(aTemperature),
		myErodedCoefficient(anErodedCoeff)
	{}

	float myElevation;
	//maybe store as short
	glm::vec3 myNormal;
	unsigned char myRainfall;
	unsigned char myTemperature;
	float myErodedCoefficient;
};

class TerrainTile
{
public:
	TerrainTile(const vec2i& aGridIndex, int aTileSize, float aResolution);
	~TerrainTile() { }
	inline const vec2i& GetGridIndex() const { return myGridIndex; }
	__forceinline void AddTerrainElement(TerrainElement anElement) { myElements.push_back(anElement); }
	__forceinline const TerrainElement GetElement(const unsigned int anIndex) const { return *(myElements.begin() + anIndex); }
	__forceinline bool IsBuilt() { return myIsBuilt; }
	__forceinline bool IsBuilding() { return myHasBuildStarted; }
	void OnStartBuild();
	void OnFinishBuild();

	__forceinline float GetTileSizeInMeters() const { return (float)myTileSize * myResolution; }

	__forceinline float GetMinHeight() const { return myMinHeight; }
	__forceinline float GetMaxHeight() const { return myMaxHeight; }

	__forceinline void SetMinHeight(float aHeight) { myMinHeight = aHeight; }
	__forceinline void SetMaxHeight(float aHeight)  { myMaxHeight = aHeight; }

	float GetY(float aX, float aZ) const;
	vec3f GetNormal(float aX, float aZ) const;

	float myTotalBuildTime;
	float myHeightmapBuildTime;
	float myErosionBuildTime;

private:
	vec2i myGridIndex;
	bool myIsBuilt;
	bool myHasBuildStarted;
	std::vector<TerrainElement> myElements;
	int myMoisture;
	int myTemperature;
	int myBiome;

	unsigned int myTileSize;
	float myResolution;

	float myMinHeight;
	float myMaxHeight;


	int myCurrentLOD;
};
