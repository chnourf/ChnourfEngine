#include "WorldGrid.h"
#include "../Core/PerlinNoise.h"
#include "../Core/Math.h"
#include <algorithm>
#include "glm\glm.hpp"
#include <iostream>
#include "../Debug/WorldGridGeneratorDebugDraw.h"

const unsigned int locPictureDimension = 512u;
const unsigned int MetersPerPixel = 128;
const float locMapSize = (float)MetersPerPixel * (float)locPictureDimension;
#ifndef NDEBUG
const unsigned int locGridNumOfElements = 128;
#else
const unsigned int locGridNumOfElements = 64;
#endif
const float locDistanceBetweenElements = locMapSize / (float)(locGridNumOfElements);
std::default_random_engine engine;
std::bernoulli_distribution boolDistribution;
std::uniform_real_distribution<float> floatDistribution(0.f, 1.f);
const unsigned int locDepth = 6u;
const float gain = 0.5f;
const float lacunarity = 1.90f;
const float locSeaLevel = 0.45f;
const float rainfallDiffusionCoefficient = 0.1f;
const float rainfallMountainTransmissionRate = 0.4f;
const float windNoisePatternSize = locMapSize / 5.f;
const unsigned int riverNumber = pow(locGridNumOfElements / 16u, 2u);
const auto midPos = Vector2<float>(locMapSize / 2.f, locMapSize / 2.f);

namespace TerrainGeneration
{
	void locAddUnique(std::vector<Point*>& aVector, Point* anObject)
	{
		if (std::find(aVector.begin(), aVector.end(), anObject) == aVector.end()) {
			aVector.push_back(anObject);
		}
	}

	WorldGrid::WorldGrid(unsigned int aSeed):
		myFloatDistribution(0.f, 1.f),
		mySeed(aSeed),
		myPerlin(aSeed)
	{
		engine.seed(mySeed);
	}

	void AddCell(Grid& aGrid, std::vector<Point*> points)
	{
		const auto pointSize = points.size();
		for (int i = 0; i < pointSize; ++i)
		{
			auto& point = points[i];
			locAddUnique(point->myNeighbours, points[(i + 1) % pointSize]);
			locAddUnique(point->myNeighbours, points[(i + pointSize - 1) % pointSize]);
		}

		aGrid.myCells.push_back(Cell(points));
	}

	std::vector<Point*> CreateRiver(Point* aSource)
	{
		Point* currentPoint = aSource;
		std::vector<Point*> riverCandidates;
		std::vector<Point*> failedCandidates;
		riverCandidates.push_back(currentPoint);

		while (true)
		{
			Point* nextPoint = nullptr;
			float lowestHeight = std::numeric_limits<float>::max();
			for (auto neighbour : currentPoint->myNeighbours)
			{
				if (riverCandidates.size() > 1 && neighbour == riverCandidates[riverCandidates.size() - 2])
				{
					continue;
				}

				if (std::find(riverCandidates.begin(), riverCandidates.end(), neighbour) != riverCandidates.end())
				{
					continue;
				}

				if (std::find(failedCandidates.begin(), failedCandidates.end(), neighbour) != failedCandidates.end())
				{
					continue;
				}

				if (neighbour->myHeight < lowestHeight)
				{
					lowestHeight = neighbour->myHeight;
					nextPoint = neighbour;
				}
			}
			if (!nextPoint)
			{
				failedCandidates.push_back(currentPoint);
				currentPoint = riverCandidates[riverCandidates.size() - 2];
				riverCandidates.erase(riverCandidates.end() - 1);
				continue;
			}

			if (nextPoint->myHeight > currentPoint->myHeight)
			{
				nextPoint->myHeight = currentPoint->myHeight - std::numeric_limits<float>::min();
			}

			riverCandidates.push_back(nextPoint);

			if ((nextPoint->myFlags & (int)PointTypeFlags::Land) == 0)
			{
				break;
			}
			else if ((nextPoint->myFlags & (int)PointTypeFlags::River) != 0)
			{
				// joined an existing river
				break;
			}
			else
			{
				currentPoint = nextPoint;
			}
		}

		std::vector<Point*> river;
		river.push_back(riverCandidates[0]);

		auto it = riverCandidates.begin() + 1;
		while (it < riverCandidates.end() - 1)
		{
			auto neighbourIt = riverCandidates.end();
			for (auto neighbour : (*it)->myNeighbours)
			{
				if (neighbour == *(it + 1) || neighbour == *(it - 1))
				{
					continue;
				}

				neighbourIt = std::find(riverCandidates.begin(), riverCandidates.end(), neighbour);
			}

			if (neighbourIt != riverCandidates.end() && neighbourIt > it)
			{
				it = neighbourIt;
			}
			else
			{
				++it;
			}
			river.push_back(*it);
		}

		for (auto point : river)
		{
			point->myFlags |= (int)PointTypeFlags::River;
		}

		return river;
	}

	void PropagateRainfall(Point* aPoint,const Vector2<float>& aWindDirection, PerlinNoise pn)
	{
		if (aPoint->myRainfall < 0.001f)
		{
			return;
		}

		// modifying wind vector to add some randomness
		Vector2<float> newWindDirection;
		float seed = M_PI * (pn.noise(aPoint->myPosition.x / windNoisePatternSize, aPoint->myPosition.y / windNoisePatternSize, 0.f) - 0.5f); // between -90 and 90 degrees variation
		newWindDirection.x = cos(seed) * aWindDirection.x - sin(seed) * aWindDirection.y;
		newWindDirection.y = sin(seed) * aWindDirection.x + cos(seed) * aWindDirection.y;

		// calculating where the rainfall will go
		std::vector<float> rainfallTransmissions;
		for (auto& neighbour : aPoint->myNeighbours)
		{
			Vector2<float> pointToNeighbour;
			pointToNeighbour.x = neighbour->myPosition.x - aPoint->myPosition.x;
			pointToNeighbour.y = neighbour->myPosition.y - aPoint->myPosition.y;
			pointToNeighbour.Normalize();

			float rainfallToTransfer = std::max(0.f, (pointToNeighbour.x * newWindDirection.x + pointToNeighbour.y * newWindDirection.y)) * (1.f - rainfallDiffusionCoefficient) + rainfallDiffusionCoefficient;
			rainfallTransmissions.push_back(rainfallToTransfer);
		}

		const float totalTransmission = std::accumulate(rainfallTransmissions.begin(), rainfallTransmissions.end(), 0.f);
		for (auto& transmission : rainfallTransmissions)
		{
			transmission /= totalTransmission;
		}

		// diffusing rainfall
		const float initialRainfall = aPoint->myRainfall;
		bool pointIsSea = (aPoint->myFlags & (int)PointTypeFlags::Land) == 0;
		for (int i = 0; i < aPoint->myNeighbours.size(); ++i)
		{
			auto& neighbour = aPoint->myNeighbours[i];

			auto landFlag = neighbour->myFlags & (int)PointTypeFlags::Land;
			bool neighbourIsSea = landFlag == 0;
			if (neighbourIsSea && pointIsSea)
			{
				continue;
			}

			Vector2<float> pointToNeighbour;
			pointToNeighbour.x = neighbour->myPosition.x - aPoint->myPosition.x;
			pointToNeighbour.y = neighbour->myPosition.y - aPoint->myPosition.y;
			pointToNeighbour.Normalize();

			float rainfallDropped = initialRainfall * rainfallTransmissions[i];
			auto mountainFlag = neighbour->myFlags & (int)PointTypeFlags::Mountain;
			if (mountainFlag != 0)
			{
				// mountains influence rainfall, we symbolize it by reducing the quantity of rainfall transmitted by an arbitrary coefficient
				rainfallDropped *= rainfallMountainTransmissionRate;
			}

			if (!pointIsSea)
			{
				//aPoint->myRainfall = std::max(aPoint->myRainfall - rainfallDropped, 0.f);
				aPoint->myRainfall -= rainfallDropped;
			}
			if (!neighbourIsSea)
			{
				neighbour->myRainfall += rainfallDropped;
			}
		}
	}

	void WorldGrid::GenerateRainfallForGrid()
	{
		const float phase = 2 * M_PI * floatDistribution(engine);
		Vector2<float> windVector;
		windVector.x = sin(phase);
		windVector.y = cos(phase);
		// don't know yet how to link iterations number and resolution
		const int iterations = locGridNumOfElements / 2;
		//const int iterations = pow(locGridNumOfElements / 16, 2);
		for (int i = 0; i < iterations; ++i)
		{
			for (auto& point : myGrid.myPoints)
			{
				PropagateRainfall(&point, windVector, myPerlin);
			}
		}
		for (auto& point : myGrid.myPoints)
		{
			auto isSea = (point.myFlags & (int)PointTypeFlags::Land) == 0;
			if (!isSea)
			{
				point.myRainfall = 5.f*point.myRainfall / (1.f + 5.f*point.myRainfall);
			}

			// rivers don't increase rainfall stricto sensu but they tend to increase moisture in the area.
			if ((point.myFlags & (int)PointTypeFlags::River) != 0)
			{
				point.myRainfall = glm::clamp(point.myRainfall + 0.3f, 0.f, 1.f);
			}
		}
		for (auto& point : myGrid.myPoints)
		{
			float rainRandomness = 0.f;
			auto warpX = locMapSize / 6.f * (myPerlin.noise(point.myPosition.x / (locMapSize / 4.f) + 0.3f, point.myPosition.y / (locMapSize / 4.f) + 0.3f, 0.f) - 0.5f);
			auto warpY = locMapSize / 6.f * (myPerlin.noise(point.myPosition.x / (locMapSize / 4.f) + 0.3f, point.myPosition.y / (locMapSize / 4.f) + 0.3f, 1.f) - 0.5f);
			for (int d = 1; d <= 4; d++)
			{
				rainRandomness += myPerlin.noise(4.f * pow(2, d) * (point.myPosition.x + warpX) / locMapSize, 4.f * pow(2, d) * (point.myPosition.y + warpY) / locMapSize, 1) / pow(2, d);
			}
			point.myRainfall = glm::clamp(point.myRainfall + (rainRandomness - 0.5f), 0.f, 1.f);
		}
	}

	void WorldGrid::GenerateRiversForGrid(const std::vector<Point*>& potentialSources)
	{
		for (int i = 0; i < riverNumber; ++i)
		{
			auto pointId = (unsigned int)(potentialSources.size() * floatDistribution(engine));
			auto& point = potentialSources[pointId];
			point->myFlags |= (int)PointTypeFlags::River;

			myGrid.myRivers.push_back(CreateRiver(point));
		}
	}

	void WorldGrid::GenerateTemperatureForGrid()
	{
		for (auto& point : myGrid.myPoints)
		{
			auto isSea = (point.myFlags & (int)PointTypeFlags::Land) == 0;
			point.myRainfall = isSea ? 1.f : 0.f;

			//point.myTemperature = 1 - abs(2.f / locMapSize * (point.myPosition.y - locMapSize / 2.f));
			point.myTemperature = sin(M_PI / locMapSize*(locMapSize - point.myPosition.y));
			float tempRandomness = 0.f;
			for (int d = 1; d <= 4; d++)
			{
				tempRandomness += myPerlin.noise(5.f * pow(2, d) * point.myPosition.x / locMapSize, 5.f * pow(2, d) * point.myPosition.y / locMapSize, 0) / pow(2, d);
			}
			point.myTemperature = glm::clamp(point.myTemperature + 0.5f * (tempRandomness - 0.5f), 0.f, 1.f);
			float altitudeInfluence = glm::clamp((point.myHeight - locSeaLevel) * 0.5f, 0.f, 1.f);
			point.myTemperature = glm::clamp(point.myTemperature - altitudeInfluence, 0.f, 1.f);
		}
	}

	void WorldGrid::Generate()
	{
		std::vector<Point*> mountainPoints;

		const unsigned int horizontalAmountOfPoints = (2 * locGridNumOfElements + 1u);
		const unsigned int verticalAmountOfPoints = (locGridNumOfElements* 4.f / 3.f + 1u);
		myGrid.myPoints.reserve(horizontalAmountOfPoints * verticalAmountOfPoints);
		const unsigned int verticalElements = verticalAmountOfPoints - 1u;
		myGrid.myPoints.reserve((locGridNumOfElements - 1u) * verticalElements);

		// placing points
		std::cout << "placing points..." << std::endl;
		for (unsigned int i = 0; i < verticalAmountOfPoints; ++i)
		{
			for (unsigned int j = 0; j < horizontalAmountOfPoints; ++j)
			{
				const auto x = (float)j * locDistanceBetweenElements / 2.f;

				float isVerticalOdd = (i % 2 != 0) ? 1.f : 0.f;
				float isHorizontalOdd = (j % 2 != 0) ? 1.f : 0.f;
				if (isVerticalOdd)
				{
					isHorizontalOdd = 1.f - isHorizontalOdd;
				}
				const auto y = ((float)i * 3.f / 4.f + 1.f / 4.f * isHorizontalOdd) * locDistanceBetweenElements;

				auto adjustedX = x;
				auto adjustedY = y;

				Vector2<float> warp = Vector2<float>(floatDistribution(engine) - .5f, floatDistribution(engine) - .5f);
				float norm = sqrt(pow(warp.x, 2) + pow(warp.y, 2));
				if (norm > 1.f)
				{
					warp.x /= norm;
					warp.y /= norm;
				}

				if (i > 0 && i < verticalAmountOfPoints - 1)
				{
					adjustedY = glm::clamp(y + warp.x * locDistanceBetweenElements / 3.f, 0.f, locMapSize);
				}
				if (j > 0 && j < horizontalAmountOfPoints - 1)
				{
					adjustedX = glm::clamp(x + warp.y * locDistanceBetweenElements / 3.f, 0.f, locMapSize);
				}

				auto pointToAdd = Point(adjustedX, adjustedY);
				myGrid.myPoints.push_back(pointToAdd);
			}
		}

		// creating hexagonal cells
		std::cout << "creating cells..." << std::endl;
		for (unsigned int i = 0; i < verticalElements; ++i)
		{
			for (unsigned int j = 0; j < locGridNumOfElements - 1; ++j)
			{
				float isVerticalEven = (i % 2 == 0) ? 1.f : 0.f;
				const auto topLeftId = 2 * j + isVerticalEven + i * horizontalAmountOfPoints;
				const auto topCenterId = (2 * j + 1 + isVerticalEven) + i * horizontalAmountOfPoints;
				const auto topRightId = (2 * j + 2 + isVerticalEven) + i * horizontalAmountOfPoints;
				const auto bottomLeftId = 2 * j + isVerticalEven + (i + 1) * horizontalAmountOfPoints;
				const auto bottomCenterId = (2 * j + 1 + isVerticalEven) + (i + 1) * horizontalAmountOfPoints;
				const auto bottomRightId = (2 * j + 2 + isVerticalEven) + (i + 1) * horizontalAmountOfPoints;

				std::vector<Point*> pointsToAdd;
				pointsToAdd.push_back(&myGrid.myPoints[topLeftId]);
				pointsToAdd.push_back(&myGrid.myPoints[topCenterId]);
				pointsToAdd.push_back(&myGrid.myPoints[topRightId]);
				pointsToAdd.push_back(&myGrid.myPoints[bottomRightId]);
				pointsToAdd.push_back(&myGrid.myPoints[bottomCenterId]);
				pointsToAdd.push_back(&myGrid.myPoints[bottomLeftId]);

				AddCell(myGrid, pointsToAdd);
			}
		}

		// ELEVATION
		std::cout << "generating height and mountains..." << std::endl;
		for (auto& point : myGrid.myPoints)
		{
			const auto& pointPos = point.myPosition;

			float tileNoise = 0.f;

			float freq = 1.f;
			float amp = 1.f;


			// points far away from the center will "sink" allowing a border ocean. Distance is artificially modified with Perlin noise to create more irregularity
			float noiseDistAttenuation = 0.f;
			for (int d = 1; d <= 6; d++)
			{
				noiseDistAttenuation += myPerlin.noise(3.f * pow(2, d) * pointPos.x / locMapSize, 3.f * pow(2, d) * pointPos.y / locMapSize, 0) / pow(2, d);
			}
			float distToCenter = (std::max(abs(pointPos.x - midPos.x), abs(pointPos.y - midPos.y))) * (1.f + noiseDistAttenuation);
			const float distAttenuation = glm::clamp(10.f / locMapSize * (0.70f * locMapSize - abs(distToCenter)), 0.f, 1.0f);

			// warped fractal Perlin noise
			auto warpX = locMapSize / 6.f * (myPerlin.noise(pointPos.x / (locMapSize / 4.f) + 0.3f, pointPos.y / (locMapSize / 4.f) + 0.3f, 0.f) - 0.5f);
			auto warpY = locMapSize / 6.f * (myPerlin.noise(pointPos.x / (locMapSize / 4.f) + 0.3f, pointPos.y / (locMapSize / 4.f) + 0.3f, 3.f) - 0.5f);
			for (int d = 1; d <= 8; d++)
			{
				freq *= lacunarity;
				amp *= gain;

				auto softNoise = 0.f;

				softNoise = myPerlin.noise(freq * (pointPos.x + warpX) / (locMapSize / 4.f), freq * (pointPos.y + warpY) / (locMapSize / 4.f), 0.f);

				tileNoise += softNoise*amp;
			}

			tileNoise *= distAttenuation;

			// MOUNTAINS
			const float locCoastalMountainsWidth = 0.04f;
			float coastalMountains = exp(-pow((tileNoise - locSeaLevel - locCoastalMountainsWidth / 2.f) / (locCoastalMountainsWidth), 2));
			float continentalMountains = 1.f / (1.f + exp(-100.f * (tileNoise - (locSeaLevel + 0.15f))));
			float someRandomNoise = 1.f / (1 + exp(-40.f * (myPerlin.noise((pointPos.x + warpX) / (locMapSize / 4.f), (pointPos.y + warpY) / (locMapSize / 10.f), 0.f) - 0.6f)));
			tileNoise += (coastalMountains + continentalMountains) * someRandomNoise;

			point.myHeight = tileNoise;
			if (tileNoise > locSeaLevel)
			{
				point.myFlags |= (int)PointTypeFlags::Land;
			}
			if (tileNoise > locSeaLevel + 0.35f)
			{
				point.myFlags |= (int)PointTypeFlags::Mountain;
				mountainPoints.push_back(&point);
			}
		}

		// TEMPERATURE
		std::cout << "computing temperature..." << std::endl;
		GenerateTemperatureForGrid();

		//RIVERS
		std::cout << "placing rivers..." << std::endl;
		GenerateRiversForGrid(mountainPoints);

		// RAINFALL
		std::cout << "diffusing rainfall..." << std::endl;
		GenerateRainfallForGrid();

		for (auto& cell : myGrid.myCells)
		{
			cell.SetBiome();
		}

#ifndef NDEBUG
		// DRAWING
		std::cout << "drawing debug images..." << std::endl;
		Debug::DrawGrid(myGrid, locPictureDimension, MetersPerPixel);
#endif
	}

	const Cell* WorldGrid::SampleGrid(const vec2f& aPosition)
	{
		const auto adjustedPosition = midPos + aPosition;

		std::vector<const Cell*> candidates;

		for (const auto& cell : myGrid.myCells)
		{
			if (IsPointInsideAABB(cell.myAABB, vec3f(adjustedPosition)))
			{
				candidates.push_back(&cell);
			}
		}

		if (candidates.size() == 0)
		{
			return nullptr;
		}

		auto currentCellId = 0u;
		for (; currentCellId < candidates.size(); ++currentCellId)
		{
			const auto& poly = candidates[currentCellId]->myPoints;
			//auto num = candidate.myPoints.size();
			auto numPoints = 6u; // optimization for hexagon
			auto inside = false;
			for (int i = 0, j = numPoints - 1; i < numPoints; ++i)
			{
				if (((poly[i]->myPosition.y > adjustedPosition.y) != (poly[j]->myPosition.y > adjustedPosition.y)) && (adjustedPosition.x < (poly[i]->myPosition.x + (poly[j]->myPosition.x - poly[i]->myPosition.x) * (adjustedPosition.y - poly[i]->myPosition.y) / (poly[j]->myPosition.y - poly[i]->myPosition.y))))
				{
					inside = !inside;
				}
				j = i;
			}

			if (inside)
			{
				break;
			}
		}

		// this is the cell where the point is
		return candidates[currentCellId];
	}
}
