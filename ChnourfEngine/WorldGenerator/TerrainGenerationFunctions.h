#pragma once
#include <vector>
#include <random>
struct TerrainElement;

namespace TerrainGeneration
{
	struct ErosionParams
	{
		float carryCapacity; // soil carry capacity
		float rockHardness; // erosion speed
		float depositionSpeed; // deposition speed
		int iterations; // number of iterations
		int depositionRadius;
		float gravity;
		float evaporation;
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

	float ComputeElevation(const float x, const float y, bool needsDetail);
	float ComputeTemperature(const float x, const float y, const float z);
	float ComputeRainfallFromGridWithPerlinNoise(const float x, const float z);

	void ComputeErosion(std::vector<TerrainElement>& elevationMap, const TerrainGeneration::ErosionParams& params, const unsigned int& aTileSize);

	void Initialize(unsigned int aSeed);

	const float seaLevel = 0.f;
}