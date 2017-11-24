#pragma once
#include <random>
#include "Geometry.h"

namespace TerrainGeneration
{
	class WorldGridGenerator
	{
	public:
		WorldGridGenerator();

		void GenerateGrid();

	private:
		Grid myGrid;

		std::default_random_engine myEngine;
		std::bernoulli_distribution myBoolDistribution;
		std::uniform_real_distribution<float> myFloatDistribution;
	};
}