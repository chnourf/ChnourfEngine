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

	void ComputeErosion(std::vector<TerrainElement>& elevationMap, const unsigned int iterations, const TerrainGeneration::ErosionParams& params, const unsigned int& aCellSize, std::default_random_engine aRandomEngine);
}