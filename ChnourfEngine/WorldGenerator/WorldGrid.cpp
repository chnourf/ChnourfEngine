#include "WorldGrid.h"
#include "../Core/PerlinNoise.h"
#include "../Core/Math.h"
#include <algorithm>
#include "glm\glm.hpp"
#include <iostream>
#include "../Debug/WorldGridGeneratorDebug.h"
#include "TerrainGenerationFunctions.h"

const unsigned int locPictureDimension{ 512 };// TerrainGeneration::GetMapTileAmount() };
const unsigned int MetersPerPixel{ unsigned(TerrainGeneration::GetMapSize()) / locPictureDimension };
#ifndef NDEBUG
const unsigned int locGridNumOfElements{ 128 };
#else
const unsigned int locGridNumOfElements{ 32 };
#endif
const float locDistanceBetweenElements{ TerrainGeneration::GetMapSize() / float(locGridNumOfElements) };
const float rainfallDiffusionCoefficient{ 0.1f };
const float rainfallMountainTransmissionRate{ 0.4f };
const float riverInfluenceOnRainfall{ 0.3f };
const float windNoisePatternSize{ TerrainGeneration::GetMapSize() / 5.f };
const unsigned int riverNumber{ unsigned(pow(locGridNumOfElements / 16u, 2u))};
const auto midPos{ Vector2<float>(TerrainGeneration::GetMapSize() / 2.f, TerrainGeneration::GetMapSize() / 2.f) };

namespace TerrainGeneration
{
	void locAddUnique(std::vector<Point*>& aVector, Point* anObject)
	{
		if (std::find(aVector.begin(), aVector.end(), anObject) == aVector.end()) {
			aVector.push_back(anObject);
		}
	}

	WorldGrid::WorldGrid(unsigned int aSeed):
		myFloatDistribution{ 0.f, 1.f },
		myPerlin{ aSeed }
	{
		myEngine.seed(aSeed);
	}

	void AddCell(Grid& aGrid, const std::vector<Point*>& points)
	{
		const auto pointSize = points.size();
		for (int i = 0; i < pointSize; ++i)
		{
			const auto& point = points[i];
			locAddUnique(point->myNeighbours, points[(i + 1) % pointSize]);
			locAddUnique(point->myNeighbours, points[(i + pointSize - 1) % pointSize]);
		}

		aGrid.myCells.push_back(Cell(points));
	}

	std::vector<Point*> CreateRiver(Point& aSource)
	{
		Point* currentPoint = &aSource;
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
				riverCandidates.pop_back();
				continue;
			}

			if (nextPoint->myHeight > currentPoint->myHeight)
			{
				nextPoint->myHeight = currentPoint->myHeight - std::numeric_limits<float>::min();
			}

			riverCandidates.push_back(nextPoint);

			if (!nextPoint->IsFlag(PointTypeFlags::Land))
			{
				break;
			}
			else if (nextPoint->IsFlag(PointTypeFlags::River))
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

		// pathfinding among all river candidates
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
			point->myFlags |= int(PointTypeFlags::River);
		}

		return river;
	}

	void PropagateRainfall(Point& aPoint, const Vector2<float>& aWindDirection, PerlinNoise pn)
	{
		if (aPoint.myRainfall < 0.001f)
		{
			return;
		}

		// modifying wind vector to add some randomness
		Vector2<float> newWindDirection;
		float angle = M_PI * (pn.noise(aPoint.myPosition.x / windNoisePatternSize, aPoint.myPosition.y / windNoisePatternSize, 0.f) - 0.5f); // between -90 and 90 degrees variation
		newWindDirection.x = cos(angle) * aWindDirection.x - sin(angle) * aWindDirection.y;
		newWindDirection.y = sin(angle) * aWindDirection.x + cos(angle) * aWindDirection.y;

		// calculating where the rainfall will go
		std::vector<float> rainfallTransmissions;
		float totalTransmission{ 0.f };
		for (auto& neighbour : aPoint.myNeighbours)
		{
			Vector2<float> pointToNeighbour{ neighbour->myPosition - aPoint.myPosition };
			pointToNeighbour.Normalize();

			const float rainfallToTransfer = glm::mix(std::max(0.f, vec2f::Dot(pointToNeighbour, newWindDirection)), 1.f, rainfallDiffusionCoefficient);
			rainfallTransmissions.push_back(rainfallToTransfer);
			totalTransmission += rainfallToTransfer;
		}

		for (auto& transmission : rainfallTransmissions)
		{
			transmission /= totalTransmission;
		}

		// diffusing rainfall
		const float initialRainfall = aPoint.myRainfall;
		bool pointIsSea = !aPoint.IsFlag(PointTypeFlags::Land);
		for (int i = 0; i < aPoint.myNeighbours.size(); ++i)
		{
			auto& neighbour = aPoint.myNeighbours[i];
			const auto neighbourIsSea = !neighbour->IsFlag(PointTypeFlags::Land);
			if (neighbourIsSea && pointIsSea)
			{
				continue;
			}

			Vector2<float> pointToNeighbour{ neighbour->myPosition - aPoint.myPosition };
			pointToNeighbour.Normalize();

			float rainfallDropped = initialRainfall * rainfallTransmissions[i];
			if (neighbour->IsFlag(PointTypeFlags::Mountain))
			{
				// mountains influence rainfall, we symbolize it by reducing the quantity of rainfall transmitted by an arbitrary coefficient
				rainfallDropped *= rainfallMountainTransmissionRate;
			}
			if (!pointIsSea)
			{
				aPoint.myRainfall -= rainfallDropped;
			}
			if (!neighbourIsSea)
			{
				neighbour->myRainfall += rainfallDropped;
			}
		}
	}

	void WorldGrid::GenerateRainfallForGrid()
	{
		const float phase{ 2.f * float(M_PI) * myFloatDistribution(myEngine) };
		Vector2<float> windVector{ sin(phase), cos(phase) };

		// intializing rainfall
		for (auto& point : myGrid.myPoints)
		{
			point.myRainfall = point.IsFlag(PointTypeFlags::Land) ? 0.f : 1.f;
		}

		// don't know yet how to link iterations number and resolution
		const int iterations = locGridNumOfElements / 2;
		//const int iterations = pow(locGridNumOfElements / 16, 2);

		for (int i = 0; i < iterations; ++i)
		{
			for (auto& point : myGrid.myPoints)
			{
				PropagateRainfall(point, windVector, myPerlin);
			}
		}

		for (auto& point : myGrid.myPoints)
		{
			// "tonemapping" rainfall values, sea points remain at max humidity
			if (point.IsFlag(PointTypeFlags::Land))
			{
				auto& rainfall = point.myRainfall;
				rainfall = 5.f * rainfall / (1.f + 5.f * rainfall);
			}

			// rivers don't increase rainfall stricto sensu but they tend to increase moisture in the area.
			if (point.IsFlag(PointTypeFlags::River))
			{
				point.myRainfall = glm::clamp(point.myRainfall + riverInfluenceOnRainfall, 0.f, 1.f);
			}

			// warping rainfall field to make it look nicer
			float rainRandomness = 0.f;
			auto warpX = TerrainGeneration::GetMapSize() / 6.f * (myPerlin.noise(point.myPosition.x / (TerrainGeneration::GetMapSize() / 4.f) + 0.3f, point.myPosition.y / (TerrainGeneration::GetMapSize() / 4.f) + 0.3f, 0.f) - 0.5f);
			auto warpY = TerrainGeneration::GetMapSize() / 6.f * (myPerlin.noise(point.myPosition.x / (TerrainGeneration::GetMapSize() / 4.f) + 0.3f, point.myPosition.y / (TerrainGeneration::GetMapSize() / 4.f) + 0.3f, 1.f) - 0.5f);
			for (int d = 1; d <= 4; d++)
			{
				rainRandomness += myPerlin.noise(4.f * pow(2, d) * (point.myPosition.x + warpX) / TerrainGeneration::GetMapSize(), 4.f * pow(2, d) * (point.myPosition.y + warpY) / TerrainGeneration::GetMapSize(), 1) / pow(2, d);
			}
			point.myRainfall = glm::clamp(point.myRainfall + (rainRandomness - 0.5f), 0.f, 1.f);
		}
	}

	void WorldGrid::GenerateRiversForGrid(const std::vector<Point*>& potentialSources)
	{
		if (potentialSources.size() != 0)
		{
			for (int i = 0; i < riverNumber; ++i)
			{
				auto pointId = unsigned(potentialSources.size() * myFloatDistribution(myEngine));
				auto& point = potentialSources[pointId];
				point->myFlags |= (int)PointTypeFlags::River;

				myGrid.myRivers.push_back(CreateRiver(*point));
			}
		}
		else
		{
			std::cout << "no potential sources, skipping river generation" << std::endl;
		}
	}

	void WorldGrid::GenerateTemperatureForGrid()
	{
		for (auto& point : myGrid.myPoints)
		{
			const auto& adjustedPost = point.myPosition - midPos;
			point.myTemperature = TerrainGeneration::ComputeTemperature(adjustedPost.x, point.myHeight, adjustedPost.y);
		}
	}

	void WorldGrid::Generate()
	{
		assert(myGrid.myPoints.capacity() == 0);

		std::vector<Point*> mountainPoints;

		const unsigned int horizontalAmountOfPoints{ 2u * locGridNumOfElements + 1u };
		const unsigned int verticalAmountOfPoints{ unsigned(locGridNumOfElements* 4.f / 3.f + 1u) };
		myGrid.myPoints.reserve(horizontalAmountOfPoints * verticalAmountOfPoints);
		const unsigned int verticalElements = verticalAmountOfPoints - 1u;
		myGrid.myPoints.reserve((locGridNumOfElements - 1u) * verticalElements);

		// placing points
		std::cout << "placing points..." << std::endl;
		for (unsigned int i = 0; i < verticalAmountOfPoints; ++i)
		{
			for (unsigned int j = 0; j < horizontalAmountOfPoints; ++j)
			{
				const auto x = float(j) * locDistanceBetweenElements / 2.f;

				float isVerticalOdd = (i % 2 != 0) ? 1.f : 0.f;
				float isHorizontalOdd = (j % 2 != 0) ? 1.f : 0.f;
				if (isVerticalOdd)
				{
					isHorizontalOdd = 1.f - isHorizontalOdd;
				}
				const auto y = (float(i) * 3.f / 4.f + 1.f / 4.f * isHorizontalOdd) * locDistanceBetweenElements;

				auto adjustedX = x;
				auto adjustedY = y;

				Vector2<float> warp = Vector2<float>(myFloatDistribution(myEngine) - .5f, myFloatDistribution(myEngine) - .5f);
				float norm = sqrt(pow(warp.x, 2) + pow(warp.y, 2));
				if (norm > 1.f)
				{
					warp.x /= norm;
					warp.y /= norm;
				}

				if (i > 0 && i < verticalAmountOfPoints - 1)
				{
					adjustedY = glm::clamp(y + warp.y * locDistanceBetweenElements / 3.f, 0.f, TerrainGeneration::GetMapSize());
				}
				if (j > 0 && j < horizontalAmountOfPoints - 1)
				{
					adjustedX = glm::clamp(x + warp.x * locDistanceBetweenElements / 3.f, 0.f, TerrainGeneration::GetMapSize());
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

				std::vector<Point*> pointsToAdd{
					&myGrid.myPoints[topLeftId],
					&myGrid.myPoints[topCenterId],
					&myGrid.myPoints[topRightId],
					&myGrid.myPoints[bottomRightId],
					&myGrid.myPoints[bottomCenterId],
					&myGrid.myPoints[bottomLeftId],
				};

				AddCell(myGrid, pointsToAdd);
			}
		}

		// ELEVATION
		std::cout << "generating height and mountains..." << std::endl;
		for (auto& point : myGrid.myPoints)
		{
			const auto& adjustedPos = point.myPosition - midPos;

			point.myHeight = TerrainGeneration::ComputeElevation(adjustedPos.x, adjustedPos.y, false);
			if (point.myHeight > 0.f)
			{
				point.myFlags |= (int)PointTypeFlags::Land;
			}
			if (point.myHeight > TerrainGeneration::GetMountainStartAltitude())
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
		Debug::DrawGrid(*this, locPictureDimension, MetersPerPixel);
#endif
	}

	bool IsInCell(const Cell& aCell, const vec2f aPosition)
	{
		const auto& poly = aCell.myPoints;
		//auto num = candidate.myPoints.size();
		auto numPoints = 6u; // optimization for hexagon
		auto inside = false;
		for (int i = 0, j = numPoints - 1; i < numPoints; ++i)
		{
			if (((poly[i]->myPosition.y > aPosition.y) != (poly[j]->myPosition.y > aPosition.y)) 
				&& (aPosition.x < (poly[i]->myPosition.x + (poly[j]->myPosition.x - poly[i]->myPosition.x) * (aPosition.y - poly[i]->myPosition.y) / (poly[j]->myPosition.y - poly[i]->myPosition.y))))
			{
				inside = !inside;
			}
			j = i;
		}

		return inside;
	}

	const Cell* WorldGrid::SampleGridCell(const vec2f& aPosition) const
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

		const Cell* result = nullptr;

		for (auto currentCellId = 0u; currentCellId < candidates.size(); ++currentCellId)
		{
			const auto currentCell = candidates[currentCellId];
			if (IsInCell(*currentCell, adjustedPosition))
			{
				result = currentCell;
				break;
			}
		}

		// this is the cell where the point is
		return result;
	}


	float WorldGrid::SampleGridRainfall(const vec2f& aPosition) const
	{
		const auto adjustedPosition = midPos + aPosition;

		// this quick test saves us a lot of time
		static thread_local const Cell* locLastFoundCell = nullptr;
		auto sampledCell = (locLastFoundCell && IsInCell(*locLastFoundCell, adjustedPosition)) ? locLastFoundCell : SampleGridCell(aPosition);

		if (!sampledCell)
		{
			return 0.f;
		}

		//assert(sampledCell);
		locLastFoundCell = sampledCell;
		float rainfall = -1.f;
		const auto center = sampledCell->GetCenter();
		Vector2<float> CenterToPosition{ adjustedPosition.x - center.x, adjustedPosition.y - center.y };
		for (int i = 0; i < sampledCell->myPoints.size(); ++i)
		{
			//taking a point and its neighbour to the right
			auto pointA = sampledCell->myPoints[i];
			auto pointB = sampledCell->myPoints[(i + 1) % (sampledCell->myPoints.size())];

			auto centerToA = Vector2<float>{ pointA->myPosition.x - center.x, pointA->myPosition.y - center.y };
			auto centerToB = Vector2<float>{ pointB->myPosition.x - center.x, pointB->myPosition.y - center.y };

			bool isInTriangle = (Vector2<float>::Cross(CenterToPosition, centerToA) <= 0.f) && (Vector2<float>::Cross(CenterToPosition, centerToB) >= 0.f);
			if (isInTriangle)
			{
				// calculate the areas and factors (order of parameters doesn't matter):
				auto posToA = Vector2<float>{ pointA->myPosition.x - adjustedPosition.x, pointA->myPosition.y - adjustedPosition.y };
				auto posToB = Vector2<float>{ pointB->myPosition.x - adjustedPosition.x, pointB->myPosition.y - adjustedPosition.y };
				auto aTotal = Vector2<float>::Cross(center - pointA->myPosition, center - pointB->myPosition); // main triangle area a
				auto aCenter = Vector2<float>::Cross(posToA, posToB); // p1's triangle area / a
				auto aA = Vector2<float>::Cross(posToB, vec2f() -CenterToPosition); // p2's triangle area / a 
				auto aB = Vector2<float>::Cross(vec2f() - CenterToPosition, posToA); // p3's triangle area / a
				// find the uv corresponding to point f (uv1/uv2/uv3 are associated to p1/p2/p3):
				rainfall = (sampledCell->GetRainfall() * aCenter + pointA->myRainfall * aA + pointB->myRainfall * aB)/ aTotal;
				break;
			}
		}

		return rainfall;
	}

	const std::vector<std::vector<Point*>>& WorldGrid::GetRivers() const
	{
		return myGrid.myRivers;
	}

}
