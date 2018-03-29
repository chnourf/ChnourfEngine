#pragma once
#include <random>
#include "Geometry.h"
#include "../Core/PerlinNoise.h"

namespace TerrainGeneration
{
	// this class is used to generate a grid of hexagonal cells that represent the whole world. This has multiples uses : compute rainfall diffusion, compute rivers, place cities and draw the map of the world for debug
	class WorldGrid
	{
	public:
		WorldGrid(unsigned int aSeed);

		void Generate();
		const Cell* SampleGridCell(const vec2f& aPosition) const;
		float SampleGridRainfall(const vec2f& aPosition) const;
		const std::vector<std::vector<Point*>>& GetRivers() const;

	private:
		Grid myGrid;

		void GenerateRainfallForGrid();
		void GenerateRiversForGrid(const std::vector<Point*>& potentialSources);
		void GenerateTemperatureForGrid();

		std::default_random_engine myEngine;
		std::bernoulli_distribution myBoolDistribution;
		std::uniform_real_distribution<float> myFloatDistribution;
		PerlinNoise myPerlin;
	};
}