#pragma once
#ifndef CELL_H
#define CELL_H

enum Biome
{
	Tundra,
	Taiga,
	Desert,
	Shrubland,
	Plains,
	Savanna,
	Swamp,
	Forest,
	RainForest,
	SeasonalForest
};

struct Cell
{
	float myDepth;
	bool isSea;
	Biome myBiome;


	Cell() :
		myDepth(0),
		isSea(false)
	{}

	Cell(float aDepth) :
		myDepth(aDepth),
		isSea(false)
	{
	}
};

#endif // !CELL_H
