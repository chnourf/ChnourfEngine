#pragma once
#include <vector>
#include <random>
struct TerrainElement;

namespace TerrainGeneration
{
	struct ErosionParams
	{
		float Kq; // soil carry capacity
		float Kevap; // evaporation speed
		float Kerosion; // erosion speed
		float Kdepos; // deposition speed
		float Ki; // inertia
		float minSlope;
		float g; // gravity
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
	float GetMountainStartAltitude();
	float GetMultiplier();

	Biome DeduceBiome(const float aTemperature, const float aRainfall);

	const char* GetBiomeName(const Biome aBiome);

	float ComputeElevation(const float x, const float y, bool needsDetail);
	float ComputeTemperature(const float x, const float y, const float z);

	void ComputeErosion(std::vector<TerrainElement>& elevationMap, const unsigned int iterations, const TerrainGeneration::ErosionParams& params, const unsigned int& aTileSize, std::default_random_engine aRandomEngine);

	void Initialize(unsigned int aSeed);
}