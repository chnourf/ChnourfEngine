#include <algorithm>
#include <iostream>
#include <time.h>
#include <shlobj.h>
#include <shlwapi.h>
#include <vector>

#include "PerlinNoise.h"
#include "ppm.h"
#include "Cell.h"
#include "math.h"

void SaveAsImage(ppm* anImage, float aData[], char* aName)
{
	for (int i = 0; i < (sizeof(aData) / sizeof(*aData)); i++)
	{
		auto n = aData[i];
		anImage->r[i] = floor(255 * n);
		anImage->g[i] = floor(255 * n);
		anImage->b[i] = floor(255 * n);
	}
	// Save the image in a binary PPM file
	anImage->write(aName);
}

Biome BiomeSelector(float temperature, float moisture) {
	moisture = moisture * temperature;

	if (temperature < 0.1)
		return Biome::Tundra;

	if (moisture < 0.2) {
		if (temperature < 0.5)
			return Biome::Tundra;
		if (temperature < 0.95)
			return Biome::Savanna;
		return Biome::Desert;
	}

	if ((moisture > 0.5) && (temperature < 0.7))
		return Biome::Swamp;

	if (temperature < 0.5)
		return Biome::Taiga;

	if (temperature < 0.97) {
		if (moisture < 0.35)
			return Biome::Shrubland;
		return Biome::Forest;
	}

	if (moisture < 0.45)
		return Biome::Plains;

	if (moisture < 0.9)
		return Biome::SeasonalForest;

	return Biome::RainForest;
}

int main(int argc, char **argv)
{
	// Define generation constants
	const unsigned int width = 1024, height = 1024;
	const unsigned int depth = 6;
	const float equator = 0.5f; // 0 is north, 1 is south
	const float centerPointX = 0.5f;
	const float centerPointY = 0.5f;

	Cell *cells = new Cell[width*height];
	float *temperature = new float[width*height];
	float *moisture = new float[width*height];
	float *lerp = new float[width*height];
	float *lerpNoises = new float[width*height];

	// Create an empty PPM image
	ppm* image = new ppm(width, height);

	// Create a PerlinNoise object with a random permutation vector generated with seed
	unsigned int seed = time(NULL);
	srand(seed);
	PerlinNoise pn(seed);

	// Visit every pixel of the image and assign a color generated with Perlin noise
	for (unsigned int i = 0; i < height; ++i) {     // y
		for (unsigned int j = 0; j < width; ++j) {  // x
			float x = (float)j / ((float)width);
			float y = (float)i / ((float)height);
			auto indice = j + i*width;
			
			auto cellNoise = 0.f;

			//sigmoid function, perhaps not the better choice
			auto lerpFactor = exp(-pow((pn.noise(4 * x, 4 * y, 0)-0.6),2)/0.004);
			//auto lerpNoise = 1 / (1 + exp(-15 * pn.noise(2 * x, 2 * y, -18) - 0.3));
			auto lerpNoise = 1/(1+exp(-15*(pn.noise(2 * x, 2 * y, -18)-0.5)));
			lerpFactor *= lerpNoise;
			lerpFactor = 1 - lerpFactor;
			//lerpFactor = lerpNoise;// 1 / (1 + exp(-5 * lerpNoise - hardNoiseRatio));
			lerp[indice] = lerpFactor;
			lerpNoises[indice] = lerpNoise;
			
			auto mountainHeight = 1;// 0.5 + 0.5* pn.noise(2 * x, 2 * y, -2);

			// Typical Perlin noise
			for (int d = 1; d <= depth; d++)
			{
				float factor = pow(2, d);
				float softNoise = pn.noise(3 * factor*x, 3 * factor*y, 0);
				auto hardNoise = (1 - abs(pn.noise(5 * factor*x, 5 * factor*y, 0) *2-1))/((d+2)/3);
				cellNoise += 1 / factor*(lerpFactor*softNoise*0.5 + hardNoise*(1-lerpFactor)*mountainHeight*0.7);
			}
			cellNoise *= (pn.noise(2 * x, 2 * y, 2)+1)/2;

			//float distance = sqrt(pow(x - centerPointX, 2) + pow(y - centerPointY, 2));
			auto newNoise = cellNoise;

			cells[indice] = Cell(newNoise);
			
			const float latitudeInfluence = (1 - pow(2*(y - equator),2))+0.2;
			temperature[indice] = clamp(latitudeInfluence-cellNoise, 0, 1);
			auto cellMoisture = 0.5 * pn.noise(10 * x, 10 * y, 1.f)+ 0.3 * pn.noise(20 * x, 20 * y, 1.f) + 0.2 * pn.noise(40 * x, 40 * y, 1.f);
			moisture[indice] = cellMoisture;

			cells[indice].myBiome = BiomeSelector(temperature[indice], cellMoisture);
		}
	}

	for (int i = 0; i < height*width; i++)
	{
		auto n = cells[i].myDepth;
		image->r[i] = floor(255 * n);
		image->g[i] = floor(255 * n);
		image->b[i] = floor(255 * n);
	}
	// Save the image in a binary PPM file
	image->write("Elevation.ppm");

	SaveAsImage(image, temperature, "Temperature.ppm");
	SaveAsImage(image, lerp, "Lerp.ppm");
	SaveAsImage(image, lerpNoises, "LerpNoise.ppm");
	SaveAsImage(image, moisture, "Moisture.ppm");

	for (int i = 0; i < height*width; i++)
	{
		float r(0), g(0), b(0);
		auto n = cells[i].myBiome;

		switch (n)
		{
		case Tundra:
			r = 0.8; g = 0.8; b = 1;
			break;
		case Taiga:
			r = 0; g = 0.5; b = 0.5;
			break;
		case Desert:
			r = 1; g = 0.8; b = 0;
			break;
		case Shrubland:
			r = 1; g = 1; b = 0.3;
			break;
		case Plains:
			r = 0.2; g = 1; b = 0.5;
			break;
		case Savanna:
			r = 1; g = 0.6; b = 0;
			break;
		case Swamp:
			r = 0; g = 0.5; b = 0.5;
			break;
		case Forest:
			r = 0; g = 1; b = 0;
			break;
		case RainForest:
			r = 0; g = 1; b = 0.3;
			break;
		case SeasonalForest:
			r = 0; g = 1; b = 0.5;
			break;
		default:
			break;
		}
		image->r[i] = floor(255 * r);
		image->g[i] = floor(255 * g);
		image->b[i] = floor(255 * b);
	}
	// Save the image in a binary PPM file
	image->write("Biomes.ppm");

	delete image;
	image = nullptr;

	delete cells;
	cells = nullptr;

	delete temperature;
	temperature = nullptr;

	delete moisture;
	moisture = nullptr;

	ShellExecute(0, 0, "d:\\Projets\\ChnourfWorldGenerator\\ChnourfWorldGenerator\\Elevation.ppm", 0, 0, SW_SHOW);
	ShellExecute(0, 0, "d:\\Projets\\ChnourfWorldGenerator\\ChnourfWorldGenerator\\Biomes.ppm", 0, 0, SW_SHOW);

	return 0;
}