#include "TerrainGenerationFunctions.h"
#include "TerrainTile.h"
#include "../Core/PerlinNoise.h"
#include <algorithm>
#include <array>
#include <vector>
#include <random>
#include "../Core/Math.h"
#include "../Core/PerlinNoise.h"

const float scale = .5f;
const float locMultiplier = 250.f / scale; // noise result between 0 and this value (in meters)
const float gain = 0.5f;
const float lacunarity = 1.90f;
const unsigned int locNoiseDepth = 8;
const unsigned int locMapSizeInTiles = 512u;
const float locMapSize = locMapSizeInTiles * 128.f; // size in meters
const float locSeaLevel = 0.45f;
const float locMountainStartAltitude = locSeaLevel + 0.35f;

PerlinNoise perlinNoise;
std::default_random_engine randomEngine;


static const char* biomeNames[static_cast<int>(TerrainGeneration::Biome::Count)] =
{
	"Snow",
	"Tundra",
	"Bare",
	"Scorched",
	"Taiga",
	"Shrubland",
	"TemperateDesert",
	"TemperateRainForest",
	"TemperateDeciduousForest",
	"Grassland",
	"TropicalRainForest",
	"SubtropicalDesert",
	"Sea",
};

namespace TerrainGeneration
{
	float GetMapSize()
	{
		return locMapSize;
	}

	unsigned int GetMapTileAmount()
	{
		return locMapSizeInTiles;
	}

	float GetMountainStartAltitude()
	{
		return (locMountainStartAltitude - locSeaLevel) * locMultiplier;
	}

	float GetMultiplier()
	{
		return locMultiplier;
	}

	Biome DeduceBiome(const float aTemperature, const float aRainfall)
	{
		if (aTemperature < 0.25f)
		{
			if (aRainfall > 0.5f) // snow
			{
				return Biome::Snow;
			}
			else if (aRainfall > 0.35f) // tundra
			{
				return Biome::Tundra;
			}
			else if (aRainfall > 0.15f) // bare
			{
				return Biome::Bare;
			}
			else // scorched
			{
				return Biome::Scorched;
			}
		}
		else if (aTemperature < 0.5f)
		{
			if (aRainfall > 0.66f) // taiga
			{
				return Biome::Taiga;
			}
			else if (aRainfall > 0.33f) // shrubland
			{
				return Biome::Shrubland;
			}
			else // temperate desert
			{
				return Biome::TemperateDesert;
			}
		}
		else if (aTemperature < 0.75)
		{
			if (aRainfall > 0.85f) // temperate rain forest
			{
				return Biome::TemperateRainForest;
			}
			else if (aRainfall > 0.5f) // temperate deciduous forest
			{
				return Biome::TemperateDeciduousForest;
			}
			else if (aRainfall > 0.15f)
			{
				return Biome::Grassland; // grassland
			}
			else
			{
				return Biome::TemperateDesert; // temperate desert
			}
		}
		else
		{
			if (aRainfall > 0.65f)
			{
				return Biome::TropicalRainForest; // tropical rain forest
			}
			else if (aRainfall > 0.35f)
			{
				return Biome::TemperateRainForest; // temperate rain forest
			}
			else if (aRainfall > 0.15f)
			{
				return Biome::Grassland; // grassland
			}
			else
			{
				return Biome::SubtropicalDesert; // subtropical desert
			}
		}
	}

	const char* GetBiomeName(const Biome aBiome)
	{
		return biomeNames[static_cast<int>(aBiome)];
	}

	float ComputeElevation(const float x, const float y, bool needsDetail)
	{
		auto elevation = 0.f;

		float lowDetailFreq = 1.f;
		float lowDetailAmp = 1.f;

		// points far away from the center will "sink" allowing a border ocean. Distance is artificially modified with Perlin noise to create more irregularity
		float noiseDistanceAttenuation = 0.f;
		for (int d = 1; d <= 6; d++)
		{
			noiseDistanceAttenuation += perlinNoise.noise(3.f * pow(2, d) * x / locMapSize, 3.f * pow(2, d) * y / locMapSize, 0) / pow(2, d);
		}
		float distToCenter = (std::max(abs(x), abs(y))) * (1.f + noiseDistanceAttenuation);
		const float distAttenuation = glm::clamp(10.f / locMapSize * (0.70f * locMapSize - abs(distToCenter)), 0.f, 1.0f);

		// warped fractal Perlin noise
		auto lowWarpX = locMapSize / 6.f * (perlinNoise.noise(x / (locMapSize / 4.f), y / (locMapSize / 4.f), 0.f) - 0.5f);
		auto lowWarpY = locMapSize / 6.f * (perlinNoise.noise(x / (locMapSize / 4.f), y / (locMapSize / 4.f), 3.f) - 0.5f);

		for (int d = 1; d <= 8; d++)
		{
			lowDetailFreq *= lacunarity;
			lowDetailAmp *= gain;

			auto softNoise = 0.f;

			softNoise = perlinNoise.noise(lowDetailFreq * (x + lowWarpX) / (locMapSize / 4.f), lowDetailFreq * (y + lowWarpY) / (locMapSize / 4.f), 0.f);

			elevation += softNoise*lowDetailAmp;
		}

		elevation *= distAttenuation;

		// MOUNTAINS RANGES
		const float locCoastalMountainsWidth = 0.08f;
		float coastalMountains = 0.8f*exp(-pow((elevation - locSeaLevel) / (locCoastalMountainsWidth), 2));
		float continentalMountains = glm::smoothstep(locSeaLevel + 0.1f, locSeaLevel + 0.2f, elevation);
		float someRandomNoise = glm::smoothstep(0.5f, 0.7f, float(perlinNoise.noise((x + lowWarpX) / (locMapSize / 10.f), (y + lowWarpY) / (locMapSize / 10.f), 0.f)));
		elevation += (coastalMountains + continentalMountains) * someRandomNoise;

		if (needsDetail)
		{
			auto lerpFactor = glm::smoothstep(locMountainStartAltitude - 0.1f, locMountainStartAltitude + 0.2f, elevation);

			// warping the mountains to mask the 8 axis of the perlin noise
			const auto warpScale = 60.f / scale;
			auto detailWarpX = warpScale * perlinNoise.noise(x*scale / 40.f, y*scale / 40.f) - 0.5f;
			auto detailWarpY = warpScale * perlinNoise.noise(x*scale / 40.f, y*scale / 40.f, 1.f) - 0.5f;

			auto hardNoiseModifier = 1.5f;
			float detailFreq = 1.f;
			float detailAmp = 0.2f;

			// Noise computation
			for (int d = 1; d <= locNoiseDepth; d++)
			{
				detailFreq *= lacunarity;
				detailAmp *= gain;
				hardNoiseModifier *= 0.85f * gain;

				auto softNoise = 0.f;
				auto hardNoise = 0.f;

				if (lerpFactor < 1.f)
				{
					softNoise = perlinNoise.noise(detailFreq * x * scale / 384.f, detailFreq * y * scale / 384.f) - 0.5f;
				}

				if (lerpFactor > 0.f)
				{
					auto n = perlinNoise.noise(detailFreq * (x + detailWarpX) * scale / 800.f, detailFreq * (y + detailWarpY) * scale / 800.f) * 2.f - 1.f;
					// C-infinity abs approximation, results between -0.5 and 0.5
					hardNoise = 0.5f - abs(60.f * n * n * n / (0.01f + 60.f * n * n));
				}

				elevation += ((1.f - lerpFactor) * softNoise * detailAmp + hardNoise * hardNoiseModifier * lerpFactor);

				detailWarpX *= 0.5f;
				detailWarpY *= 0.5f;
			}
		}

		elevation -= locSeaLevel;
		elevation *= locMultiplier;

		// Water level is at 0.f
		return elevation;
	}

	float ComputeTemperature(const float x, const float y, const float z)
	{
		float temperature = cos(M_PI / locMapSize * z);
		float tempRandomness = 0.f;
		for (int d = 1; d <= 4; d++)
		{
			tempRandomness += perlinNoise.noise(5.f * pow(2, d) * x / locMapSize, 5.f * pow(2, d) * z / locMapSize, 0) / pow(2, d);
		}
		temperature = glm::clamp(temperature + 0.5f * (tempRandomness - 0.5f), 0.f, 1.f);
		float altitudeInfluence = glm::clamp(0.5f * y / locMultiplier , 0.f, 1.f);
		temperature = glm::clamp(temperature - 0.5f * altitudeInfluence, 0.f, 1.f);

		return temperature;
	}

	void Depose(std::vector<TerrainElement>& elevationMap, const TerrainGeneration::ErosionParams& params, const unsigned int aTileSize, const float amount, float& carriedSediment, const int xi, const int zi, const float xp, const float zp)
	{
		for (int z = zi - params.depositionRadius; z <= zi + params.depositionRadius; ++z)
		{
			if (z < 0 || z > aTileSize - 1)
			{
				continue;
			}
			float zo = z - zp;
			float zo2 = zo*zo;
			for (int x = xi - params.depositionRadius; x <= xi + params.depositionRadius; ++x)
			{
				if (x < 0 || x > aTileSize - 1)
				{
					continue;
				}
				float xo = x - xp;
				float weight = 1.f / ((1.f + float(params.depositionRadius)) * (1.f + (xo*xo + zo2)));
				elevationMap[x + aTileSize * z].myElevation += amount * weight;
				auto& erodedCoeff = elevationMap[x + aTileSize * z].myErodedCoefficient;
				erodedCoeff = (erodedCoeff + (10.f * abs(amount))) / (1.f + (erodedCoeff + (10.f * abs(amount))));
			}
		}

		carriedSediment -= amount;
	}

	void ComputeErosion(std::vector<TerrainElement>& elevationMap, const TerrainGeneration::ErosionParams& params, const unsigned int& aTileSize)
	{
		std::uniform_int_distribution<int> distribution(0, aTileSize - 2);

		const unsigned int MAX_PATH_LEN = aTileSize * 4;

		for (unsigned int iter = 0; iter < params.iterations; ++iter)
		{
			int xi = distribution(randomEngine);
			int zi = distribution(randomEngine);

			float xPos = xi, zPos = zi;

			float currentHeight = elevationMap[xi + aTileSize * zi].myElevation;
			float carriedSediment = 0.f;

			float height00 = currentHeight;
			float height10 = elevationMap[xi + 1 + aTileSize * zi].myElevation;
			float height01 = elevationMap[xi + aTileSize * (zi + 1)].myElevation;
			float height11 = elevationMap[(xi + 1) + aTileSize * (zi + 1)].myElevation;

			float deltaX = 0, deltaZ = 0;

			for (unsigned int numMoves = 0; numMoves < MAX_PATH_LEN; ++numMoves)
			{
				// calc gradient
				float gradX = height00 + height01 - height10 - height11;
				float gradZ = height00 + height10 - height01 - height11;

				// calc next pos
				deltaX = gradX;
				deltaZ = gradZ;

				float deltaLength = sqrtf(deltaX*deltaX + deltaZ*deltaZ);
				if (deltaLength <= FLT_EPSILON)
				{
					// pick random dir
					float a = std::rand();
					deltaX = cosf(a);
					deltaZ = sinf(a);
				}
				else
				{
					deltaX /= deltaLength;
					deltaZ /= deltaLength;
				}

				float newXpos = xPos + deltaX;
				float newZpos = zPos + deltaZ;

				// sample next height
				int newXi = glm::clamp(int(std::floor(newXpos)), 0, int(aTileSize) - 1);
				int newZi = glm::clamp(int(std::floor(newZpos)), 0, int(aTileSize) - 1);

				// the drop falls off the tile
				if (newXi == aTileSize - 1 || newZi == aTileSize - 1 || newXi == 0 || newZi == 0)
					break;

				float newXf = newXpos - newXi;
				float newZf = newZpos - newZi;

				float newHeight00 = elevationMap[newXi + aTileSize * newZi].myElevation;
				float newHeight10 = elevationMap[newXi + 1 + aTileSize * newZi].myElevation;
				float newHeight01 = elevationMap[newXi + aTileSize * (newZi + 1)].myElevation;
				float newHeight11 = elevationMap[newXi + 1 + aTileSize * (newZi + 1)].myElevation;

				float newHeight = (newHeight00*(1 - newXf) + newHeight10 * newXf) * (1 - newZf) + (newHeight01 * (1 - newXf) + newHeight11 * newXf) * newZf;

				float deltaHeight = currentHeight - newHeight + 0.001f;

				// if higher than current, try to deposit sediment up to neighbour height
				if (deltaHeight < 0.f)
				{
					if (-deltaHeight >= carriedSediment)
					{
						// deposit all sediment and stop
						Depose(elevationMap, params, aTileSize, carriedSediment, carriedSediment, xi, zi, xPos, zPos);
						break;
					}
					Depose(elevationMap, params, aTileSize, -deltaHeight, carriedSediment, xi, zi, xPos, zPos);
				}
				else
				{
					// compute transport capacity
					float sedimentEroded = deltaHeight * (1.f - params.rockHardness);
					carriedSediment += sedimentEroded;
					//float sedimentExcipient = std::max(0.f, carriedSediment + sedimentEroded - params.carryCapacity);
					float sedimentExcipient = std::max(0.f, carriedSediment - params.carryCapacity);

					// deposit/erode (don't erode more than dh)
					if (sedimentExcipient > 0.f)
					{
						Depose(elevationMap, params, aTileSize, sedimentExcipient, carriedSediment, xi, zi, xPos, zPos);
					}
					else
					{
						// erode (deposing a negative amount)
						Depose(elevationMap, params, aTileSize, -sedimentEroded, carriedSediment, xi, zi, xPos, zPos);
					}
				}

				// move to the neighbour
				xPos = newXpos; zPos = newZpos;
				xi = newXi; zi = newZi;

				currentHeight = newHeight;
				height00 = newHeight00;
				height10 = newHeight10;
				height01 = newHeight01;
				height11 = newHeight11;
			}
		}
	}

	void Initialize(unsigned int aSeed)
	{
		perlinNoise = PerlinNoise(aSeed);
		randomEngine.seed(aSeed);
	}

}