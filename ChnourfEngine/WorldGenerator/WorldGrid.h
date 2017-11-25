#pragma once
#include <random>
#include "Geometry.h"
#include "../Core/PerlinNoise.h"

namespace TerrainGeneration
{
	class WorldGrid
	{
	public:
		WorldGrid(unsigned int aSeed);

		void Generate();
		const Cell* SampleGrid(const vec2f& aPosition);

	private:
		Grid myGrid;

		void GenerateRainfallForGrid();
		void GenerateRiversForGrid(const std::vector<Point*>& potentialSources);
		void GenerateTemperatureForGrid();

		std::default_random_engine myEngine;
		std::bernoulli_distribution myBoolDistribution;
		std::uniform_real_distribution<float> myFloatDistribution;
		unsigned int mySeed;
		PerlinNoise myPerlin;
	};
}