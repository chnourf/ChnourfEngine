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

	enum class LandscapeType
	{
		Sea,
		Plain,
		Hills,
		Mountains,
		HighMountains,
		Count,
	};
	
	Biome GetBiome(const float aTemperature, const float aRainfall);

	void ComputeErosion(std::vector<TerrainElement>& elevationMap, const unsigned int iterations, const TerrainGeneration::ErosionParams& params, const unsigned int& aCellSize, std::default_random_engine aRandomEngine);

	void SetLandscapeRepartitionConstants(const std::array<float,4> anArray);
}