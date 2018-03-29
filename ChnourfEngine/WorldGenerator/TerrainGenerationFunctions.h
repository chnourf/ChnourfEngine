#pragma once
#include <vector>
#include "../Core/Vector.h"
#include <array>
#include <random>
struct TerrainElement;

namespace TerrainGeneration
{
	struct ErosionParams
	{
		float deltaTime;
		float maxErosionDepth;
		float pipeArea;
		float waterRainfall;
		float carryCapacity; // soil carry capacity
		float rockHardness; // erosion speed
		float soilSuspensionRate; // deposition speed
		float minAngle;
		int iterations; // number of iterations
		float gravity;
		float evaporation;
		float talusAngle;
		float thermalErosionRate;
	};

	struct ErosionData
	{
		float elevation = 0.f;
		float water = 0.f;
		float sediment = 0.f;
		std::array<float, 4> outputFlow = { 0.f, 0.f, 0.f, 0.f };
		vec2f velocity;
		float movedSediment = 0.f;
		float rockSoftness = 0.f;
	};

	enum class Biome
	{
		Snow,
		Tundra,
		Bare,
		Scorched,
		Taiga,
		Shrubland,
		TemperateDesert,
		TemperateRainForest,
		TemperateDeciduousForest,
		Grassland,
		TropicalRainForest,
		SubtropicalDesert,
		Sea,
		Count,
		Invalid = -1,
	};

	enum class LandscapeType
	{
		Sea,
		Plain,
		Hills,
		Mountains,
		HighMountains,
		Count,
	};

	float GetMapSize();
	unsigned int GetMapTileAmount();
	float GetMountainStartAltitude();
	float GetMultiplier();

	Biome DeduceBiome(const float aTemperature, const float aRainfall);

	const char* GetBiomeName(const Biome aBiome);

	float ComputeElevation(const float x, const float y, const bool needsDetail);
	float ComputeTemperature(const float x, const float y, const float z);
	float ComputeRainfallFromGridWithPerlinNoise(const float x, const float z);

	//void ComputeErosion(std::vector<TerrainElement>& elevationMap, const TerrainGeneration::ErosionParams& params, const unsigned int& aTileSize);
	void ComputeErosionNew(std::vector<ErosionData>& cellData, const TerrainGeneration::ErosionParams& params, const unsigned int& aTileSize, const float aTileResolution);

	void Initialize(const unsigned int aSeed);

	const float seaLevel = 0.f;
}