#include <algorithm>
#include <array>
#include <vector>
#include <random>
#include "TerrainGenerationFunctions.h"
#include "TerrainTile.h"
#include "../Core/PerlinNoise.h"
#include "../Core/Math.h"
#include "../Core/PerlinNoise.h"
#include "../WorldGenerator/TerrainManager.h"

const float scale = .5f;
const float locMultiplier = 250.f / scale; // noise result between 0 and this value (in meters)
const float gain = 0.5f;
const float lacunarity = 1.90f;
const unsigned int locNoiseDepth = 8;
const unsigned int locMapSizeInTiles = 512u;
const float locMapSize = locMapSizeInTiles * 128.f; // size in meters
const float locSeaLevel = 0.45f;
const float locMountainRelativeStartAltitude = locSeaLevel + 0.35f;
const float locMountainAbsoluteStartAltitude = (locMountainRelativeStartAltitude - locSeaLevel) * locMultiplier;

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
		return locMountainAbsoluteStartAltitude;
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
		else if (aTemperature < 0.75f)
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

			const auto softNoise = perlinNoise.noise(lowDetailFreq * (x + lowWarpX) / (locMapSize / 4.f), lowDetailFreq * (y + lowWarpY) / (locMapSize / 4.f), 0.f);

			elevation += softNoise * lowDetailAmp;
		}

		elevation *= distAttenuation;

		// MOUNTAINS RANGES
		static const float locCoastalMountainsWidth = 0.08f;
		float coastalMountains = 0.8f*exp(-pow((elevation - locSeaLevel) / (locCoastalMountainsWidth), 2));
		float continentalMountains = glm::smoothstep(locSeaLevel + 0.1f, locSeaLevel + 0.2f, elevation);
		float someRandomNoise = glm::smoothstep(0.5f, 0.7f, float(perlinNoise.noise((x + lowWarpX) / (locMapSize / 10.f), (y + lowWarpY) / (locMapSize / 10.f), 0.f)));
		elevation += (coastalMountains + continentalMountains) * someRandomNoise;

		if (needsDetail)
		{
			static const float mountainRelativeStartAltitudeLowRange = locMountainRelativeStartAltitude - 0.1f;
			static const float mountainRelativeStartAltitudeHighRange = locMountainRelativeStartAltitude + 0.2f;
			const auto lerpFactor = glm::smoothstep(mountainRelativeStartAltitudeLowRange, mountainRelativeStartAltitudeHighRange, elevation);

			// warping the mountains to mask the 8 axis of the perlin noise
			const auto warpScale = 60.f / scale;
			auto detailWarpX = warpScale * perlinNoise.noise(x*scale / 40.f, y*scale / 40.f) - 0.5f;
			auto detailWarpY = warpScale * perlinNoise.noise(x*scale / 40.f, y*scale / 40.f, 1.f) - 0.5f;

			auto hardNoiseModifier = 1.f;
			float detailFreq = 1.f;
			float detailAmp = 0.2f;
			const float seaCoastMultiplier = glm::clamp(50.f * (elevation - locSeaLevel), 0.f, 1.f); // smooth coasts by disabling detail noise for better shoreline

			if (seaCoastMultiplier > 0.f)
			{
				// Noise computation
				for (int d = 1; d <= locNoiseDepth; d++)
				{
					detailFreq *= lacunarity;
					detailAmp *= gain;
					hardNoiseModifier *= 0.9f * gain;

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

					elevation += seaCoastMultiplier* (((1.f - lerpFactor) * softNoise * detailAmp + hardNoise * hardNoiseModifier * lerpFactor));

					detailWarpX *= 0.5f;
					detailWarpY *= 0.5f;
				}
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
		const float altitudeInfluence = glm::clamp(0.5f * y / locMultiplier , 0.f, 1.f);
		temperature = glm::clamp(temperature - 0.5f * altitudeInfluence, 0.f, 1.f);

		return temperature;
	}

	float ComputeRainfallFromGridWithPerlinNoise(const float x, const float z)
	{
		auto rainfall = Manager::TerrainManager::GetInstance()->SampleRainfallFromGrid(vec2f(x, z));

		// high frequency perlin noise
		for (int d = 1; d <= 2; d++)
		{
			rainfall += .2f * (perlinNoise.noise(500.f * pow(2, d) * x / locMapSize, 500.f * pow(2, d) * z / locMapSize, 0) - 0.5f) / pow(2, d);
		}

		return glm::clamp(rainfall, 0.f, 1.f);
	}

	//void Erode(std::vector<TerrainElement>& elevationMap, const TerrainGeneration::ErosionParams& params, const unsigned int aTileSize, const float erodedSediment, float& carriedSediment, const float xp, const float zp)
	//{
	//	for (int z = floor(zp) - params.depositionRadius; z <= floor(zp) + 1 + params.depositionRadius; ++z)
	//	{
	//		const auto OnePlusSquaredDepositionRadius = 1.f + float(params.depositionRadius) * float(params.depositionRadius);

	//		if (z < 0 || z > aTileSize - 1)
	//		{
	//			continue;
	//		}
	//		const float zo = z - zp;
	//		const float zo2 = zo*zo;
	//		for (int x = floor(xp) - params.depositionRadius; x <= floor(xp) + 1 + params.depositionRadius; ++x)
	//		{
	//			if (x < 0 || x > aTileSize - 1)
	//			{
	//				continue;
	//			}
	//			float xo = x - xp;
	//			float weight = 1.f / (OnePlusSquaredDepositionRadius * (1.f + (xo*xo + zo2)));
	//			auto& element = elevationMap[x + aTileSize * z];
	//			element.myElevation -= erodedSediment * weight;
	//			auto& erodedCoeff = element.myErodedCoefficient;
	//			erodedCoeff = std::min(0.05f * abs(erodedSediment) + erodedCoeff, 1.f);
	//		}
	//	}

	//	carriedSediment += erodedSediment;
	//}

	//void ComputeErosion(std::vector<TerrainElement>& elevationMap, const TerrainGeneration::ErosionParams& params, const unsigned int& aTileSize)
	//{
	//	std::uniform_int_distribution<int> distribution{ 0, int(aTileSize) - 2 };

	//	const unsigned int MAX_PATH_LEN = aTileSize * 4;

	//	for (unsigned int iter = 0u; iter < params.iterations; ++iter)
	//	{
	//		int xi = distribution(randomEngine);
	//		int zi = distribution(randomEngine);

	//		float xPos = xi, zPos = zi;

	//		float currentHeight = elevationMap[xi + aTileSize * zi].myElevation;
	//		if (currentHeight < seaLevel)
	//		{
	//			continue;
	//		}
	//		float carriedSediment = 0.f;
	//		float speed = 0.f;
	//		float water = 1.f;

	//		float height00 = currentHeight;
	//		float height10 = elevationMap[xi + 1 + aTileSize * zi].myElevation;
	//		float height01 = elevationMap[xi + aTileSize * (zi + 1)].myElevation;
	//		float height11 = elevationMap[(xi + 1) + aTileSize * (zi + 1)].myElevation;

	//		float deltaX = 0.f, deltaZ = 0.f;

	//		for (unsigned int numMoves = 0; numMoves < MAX_PATH_LEN; ++numMoves)
	//		{
	//			// calc gradient
	//			float gradX = height00 + height01 - height10 - height11;
	//			float gradZ = height00 + height10 - height01 - height11;

	//			// calc next pos
	//			deltaX = gradX;
	//			deltaZ = gradZ;

	//			float deltaLength = sqrtf(deltaX*deltaX + deltaZ*deltaZ);
	//			if (deltaLength <= FLT_EPSILON)
	//			{
	//				// pick random dir
	//				float a = std::rand() * float(M_PI);
	//				deltaX = cosf(a);
	//				deltaZ = sinf(a);
	//			}
	//			else
	//			{
	//				deltaX /= deltaLength;
	//				deltaZ /= deltaLength;
	//			}

	//			float newXpos = xPos + deltaX;
	//			float newZpos = zPos + deltaZ;

	//			// sample next height
	//			int newXi = glm::clamp(int(std::floor(newXpos)), 0, int(aTileSize) - 1);
	//			int newZi = glm::clamp(int(std::floor(newZpos)), 0, int(aTileSize) - 1);

	//			// the drop falls off the tile
	//			if (water < 0.0001f || newXi == aTileSize - 1 || newZi == aTileSize - 1 || newXi == 0 || newZi == 0)
	//			{
	//				Erode(elevationMap, params, aTileSize, -carriedSediment, carriedSediment, newXpos, newZpos);
	//				break;
	//			}

	//			float newXf = newXpos - newXi;
	//			float newZf = newZpos - newZi;

	//			float newHeight00 = elevationMap[newXi + aTileSize * newZi].myElevation;
	//			float newHeight10 = elevationMap[newXi + 1 + aTileSize * newZi].myElevation;
	//			float newHeight01 = elevationMap[newXi + aTileSize * (newZi + 1)].myElevation;
	//			float newHeight11 = elevationMap[newXi + 1 + aTileSize * (newZi + 1)].myElevation;

	//			float newHeight = (newHeight00*(1 - newXf) + newHeight10 * newXf) * (1 - newZf) + (newHeight01 * (1 - newXf) + newHeight11 * newXf) * newZf;

	//			float deltaHeight = currentHeight - newHeight;

	//			// if higher than current, try to deposit sediment up to neighbour height
	//			if (deltaHeight < 0.f)
	//			{
	//				if (-deltaHeight >= carriedSediment)
	//				{
	//					// deposit all sediment and stop
	//					Erode(elevationMap, params, aTileSize, -carriedSediment, carriedSediment, xPos, zPos);
	//					break;
	//				}
	//				Erode(elevationMap, params, aTileSize, deltaHeight, carriedSediment, xPos, zPos);
	//			
	//			}
	//			else
	//			{
	//				speed = sqrt(speed * speed + params.gravity * deltaHeight);

	//				// compute transport capacity
	//				const float carryCapacity = std::max(deltaHeight, 0.01f) * speed * water * params.carryCapacity;
	//				const float sedimentExcipient = carriedSediment - carryCapacity;

	//				// deposit/erode (don't erode more than dh)
	//				if (sedimentExcipient > 0.f)
	//				{
	//					Erode(elevationMap, params, aTileSize, -sedimentExcipient * params.depositionSpeed, carriedSediment, xPos, zPos);
	//				}
	//				else
	//				{
	//					auto sedimentEroded = std::min(deltaHeight, -sedimentExcipient * (1.f - params.rockHardness));
	//					Erode(elevationMap, params, aTileSize, sedimentEroded, carriedSediment, xPos, zPos);
	//				}
	//			}

	//			// move to the neighbour
	//			xPos = newXpos; zPos = newZpos;
	//			xi = newXi; zi = newZi;

	//			currentHeight = newHeight;
	//			height00 = newHeight00;
	//			height10 = newHeight10;
	//			height01 = newHeight01;
	//			height11 = newHeight11;

	//			water *= (1.f - params.evaporation);
	//		}
	//	}
	//}

	enum
	{
		left,
		right,
		top,
		bottom
	};

	// https://helloacm.com
	inline float
		BilinearInterpolation(float q11, float q12, float q21, float q22, float x, float y, float x1 = 0.f, float x2 = 1.f, float y1 = 0.f, float y2 = 1.f)
	{
		float x2x1, y2y1, x2x, y2y, yy1, xx1;
		x2x1 = x2 - x1;
		y2y1 = y2 - y1;
		x2x = x2 - x;
		y2y = y2 - y;
		yy1 = y - y1;
		xx1 = x - x1;
		return 1.0 / (x2x1 * y2y1) * (
			q11 * x2x * y2y +
			q21 * xx1 * y2y +
			q12 * x2x * yy1 +
			q22 * xx1 * yy1
			);
	}

	void ComputeErosionNew(std::vector<ErosionData>& cellData, const TerrainGeneration::ErosionParams& params, const unsigned int& aTileSize, const float aTileResolution)
	{
		//const auto thermalErosionRate = 0.15f;
		//const auto talusTan = 0.8f;

		assert(cellData.size() == aTileSize * aTileSize);

		for (auto iteration = 0; iteration < params.iterations; ++iteration)
		{
			const auto oldState = cellData; // perform copy

			for (unsigned i = 1; i < aTileSize - 1u; ++i)
			{
				for (unsigned j = 1; j < aTileSize - 1u; ++j)
				{
					const auto index = i + j * aTileSize;
					auto& newElement = cellData[index];
					const auto& element = oldState[index];
					const auto& leftElement = oldState[index - 1];
					const auto& rightElement = oldState[index + 1];
					const auto& topElement = oldState[index - aTileSize];
					const auto& bottomElement = oldState[index + aTileSize];

					const auto& topLeftElement = oldState[index - 1 - aTileSize];
					const auto& topRightElement = oldState[index + 1 - aTileSize];
					const auto& bottomLeftElement = oldState[index + aTileSize - 1];
					const auto& bottomRightElement = oldState[index + aTileSize + 1];

					// adding rain water -----------------------------------------------------------------------------------------------
					newElement.water += params.deltaTime * params.waterRainfall;

					// compute flow between element -----------------------------------------------------------------------------------------------
					auto deltaHeightLeft = element.elevation + element.water - leftElement.elevation - leftElement.water;
					auto outputFlowLeft = std::max(0.f, element.outputFlow[left] + params.deltaTime * deltaHeightLeft * params.gravity * params.pipeArea / aTileResolution);
					auto deltaHeightRight = element.elevation + element.water - rightElement.elevation - rightElement.water;
					auto outputFlowRight = std::max(0.f, element.outputFlow[right] + params.deltaTime * deltaHeightRight * params.gravity * params.pipeArea / aTileResolution);
					auto deltaHeightTop = element.elevation + element.water - topElement.elevation - topElement.water;
					auto outputFlowTop = std::max(0.f, element.outputFlow[top] + params.deltaTime * deltaHeightTop * params.gravity * params.pipeArea / aTileResolution);
					auto deltaHeightBottom = element.elevation + element.water - bottomElement.elevation - bottomElement.water;
					auto outputFlowBottom = std::max(0.f, element.outputFlow[bottom] + params.deltaTime * deltaHeightBottom * params.gravity * params.pipeArea / aTileResolution);

					newElement.outputFlow[left] =  outputFlowLeft;
					newElement.outputFlow[right] =  outputFlowRight;
					newElement.outputFlow[top] =  outputFlowTop;
					newElement.outputFlow[bottom] = outputFlowBottom;

					if ((outputFlowLeft + outputFlowRight + outputFlowTop + outputFlowBottom) > 0.f)
					{
						auto conservationFactor = std::min(1.f, element.water * aTileResolution * aTileResolution / (params.deltaTime * (outputFlowLeft + outputFlowRight + outputFlowTop + outputFlowBottom)));

						newElement.outputFlow[left] *= conservationFactor ;
						newElement.outputFlow[right] *= conservationFactor ;
						newElement.outputFlow[top] *= conservationFactor ;
						newElement.outputFlow[bottom] *= conservationFactor ;
					}

					// update water level -----------------------------------------------------------------------------------------------
					const float deltaV = params.deltaTime * (leftElement.outputFlow[right] + rightElement.outputFlow[left] + topElement.outputFlow[bottom] + bottomElement.outputFlow[top] - std::accumulate(newElement.outputFlow.begin(), newElement.outputFlow.end(), 0.f));
					assert(newElement.water >= 0.f);
					newElement.water += deltaV / (aTileResolution * aTileResolution);
					assert(newElement.water >= 0.f);

					// update velocity --------------------------------------------------------------------------------------------------
					const float deltaWx = 0.5f * (leftElement.outputFlow[right] - element.outputFlow[left] + element.outputFlow[right] - rightElement.outputFlow[left]);
					const float deltaWy = 0.5f * (topElement.outputFlow[bottom] - element.outputFlow[top] + element.outputFlow[bottom] - bottomElement.outputFlow[top]);
					newElement.velocity = vec2f(deltaWx, deltaWy);

					// thermal erosion --------------------------------------------------------------------------------------------------
					//const std::array<std::pair<const ErosionData&, float>, 8> neighbours{
					//	std::pair<const ErosionData&, float>(topLeftElement, sqrt(2.f) * aTileResolution),
					//	std::pair<const ErosionData&, float>(topElement, aTileResolution),
					//	std::pair<const ErosionData&, float>(topRightElement, sqrt(2.f) * aTileResolution),
					//	std::pair<const ErosionData&, float>(leftElement, aTileResolution),
					//	std::pair<const ErosionData&, float>(rightElement, aTileResolution),
					//	std::pair<const ErosionData&, float>(bottomLeftElement, sqrt(2.f) * aTileResolution),
					//	std::pair<const ErosionData&, float>(bottomElement, aTileResolution),
					//	std::pair<const ErosionData&, float>(bottomRightElement, sqrt(2.f) * aTileResolution) };

					//std::vector<ErosionData&> affectedElements;
					//auto highestSlope = 0.f;
					//for (auto neighbour : neighbours)
					//{
					//	const auto slope = element.elevation - neighbour.first.elevation;
					//	if (slope > highestSlope)
					//	{
					//		highestSlope = slope;
					//	}

					//	if (slope > 0.f && (slope / neighbour.second > talusTan))
					//	{
					//		affectedElements.push_back();
					//	}
					//}

					//const auto sedimentErodedThermalErosion = aTileResolution * aTileResolution * deltaTime * thermalErosionRate * element.rockSoftness * highestSlope * 0.5f;

					// compute carry capacity -------------------------------------------------------------------------------------------
					const auto delta = 1.f / (float(aTileSize) - 1.f);
					const auto va = glm::normalize(glm::vec3(2 * delta, (rightElement.elevation - leftElement.elevation) / (aTileSize * aTileResolution), 0.0f));
					const auto vb = glm::normalize(glm::vec3(0.0f, (bottomElement.elevation - topElement.elevation) / (aTileSize * aTileResolution), 2 * delta));
					const auto normalglm = glm::cross(vb, va);
					const auto normal = vec3f(normalglm.x, normalglm.y, normalglm.z);

					const auto carryCapacity = params.carryCapacity * std::max(params.minAngle, sqrt(1.f - pow(normal.y, 2))) * element.velocity.Norm() * glm::clamp(1.f - element.water / params.maxErosionDepth, 0.f, 1.f);

					if (carryCapacity >= element.sediment)
					{
						auto erodedSediment = params.deltaTime * element.rockSoftness * (carryCapacity - element.sediment);
						newElement.elevation -= erodedSediment;
						newElement.sediment += erodedSediment;
						newElement.water += erodedSediment;
						assert(newElement.water >= 0.f);
						//newElement.movedSediment += erodedSediment;
						//newElement.rockSoftness = std::min(0.9f, element.rockSoftness + erodedSediment * 5.f);
					}
					else
					{
						auto depositedSediment = std::min(params.deltaTime * (element.sediment - carryCapacity) * params.soilSuspensionRate, newElement.water);
						assert(depositedSediment >= 0.f);
						newElement.elevation += 10.f*depositedSediment;
						newElement.sediment -= depositedSediment;
						newElement.water -= depositedSediment;
						newElement.movedSediment += depositedSediment;
					}

				}
			}

			for (unsigned i = 1; i < aTileSize - 1u; ++i)
			{
				for (unsigned j = 1; j < aTileSize - 1u; ++j)
				{
					const auto index = i + j * aTileSize;
					auto& newElement = cellData[index];

					// moving sediments
					auto backwardPoint = vec2f{
						glm::clamp(float(i) - newElement.velocity.x * params.deltaTime * 10.f, 0.f, float(aTileSize) - 1.00001f),
						glm::clamp(float(j) - newElement.velocity.y * params.deltaTime * 10.f, 0.f, float(aTileSize) - 1.00001f) };

					auto backwardFloor = vec2i(int(backwardPoint.x), int(backwardPoint.y));

					const auto& topLeftSediment = cellData[backwardFloor.x + backwardFloor.y * aTileSize].sediment;
					const auto& topRightSediment = cellData[(backwardFloor.x + 1) + backwardFloor.y * aTileSize].sediment;
					const auto& bottomLeftSediment = cellData[backwardFloor.x + (backwardFloor.y + 1) * aTileSize].sediment;
					const auto& bottomRightSediment = cellData[(backwardFloor.x + 1) + (backwardFloor.y + 1) * aTileSize].sediment;

					newElement.sediment = BilinearInterpolation(bottomLeftSediment, topLeftSediment, bottomRightSediment, topRightSediment, backwardPoint.x - float(backwardFloor.x), backwardPoint.y - float(backwardFloor.y));

					// evaporating water
					newElement.water *= (1.f - params.evaporation * params.deltaTime);
				}
			}
		}
		
	}

	void Initialize(unsigned int aSeed)
	{
		perlinNoise = PerlinNoise(aSeed);
		randomEngine.seed(aSeed);
	}

}