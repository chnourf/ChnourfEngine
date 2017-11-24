#pragma once

#include <vector>
#include <numeric>
#include "../Core/Vector.h"

namespace TerrainGeneration
{
	enum class PointTypeFlags
	{
		Land = 1 << 0,
		Mountain = 1 << 1,
		River = 1 << 2,
	};

	struct Point
	{
		Point()
		{
			myPosition = Vector2<float>();
		}

		Point(const float x, const float y)
		{
			myPosition.x = x;
			myPosition.y = y;
			myNeighbours.reserve(3); // for an hexagonal grid
			myFlags = 0;
			myRainfall = 0.f;
			myTemperature = 0.f;
		}

		Vector2<float> myPosition;
		std::vector<Point*> myNeighbours;
		int myFlags;
		float myTemperature;
		float myRainfall;
		float myHeight;
	};

	struct Triangle
	{
		Triangle() :
			myA(nullptr),
			myB(nullptr),
			myC(nullptr)
		{}

		Triangle(Point* aA, Point* aB, Point* aC)
		{
			myA = aA;
			myB = aB;
			myC = aC;
		}

		Point* myA;
		Point* myB;
		Point* myC;
	};

	struct Cell
	{
		Cell(std::vector<Point*> aPoints) :
			myPoints(aPoints) {}

		std::vector<Point*> myPoints;

		inline Vector2<float> GetCenter() const
		{
			Vector2<float> center = Vector2<float>(0.f, 0.f);
			for (auto point : myPoints)
			{
				center.x += point->myPosition.x;
				center.y += point->myPosition.y;
			}

			center.x /= myPoints.size();
			center.y /= myPoints.size();

			return center;
		}

		inline float GetElevation() const
		{
			float sum = std::accumulate(myPoints.begin(), myPoints.end(), 0.0f, [](float currentSum, Point* aPoint) {return currentSum + aPoint->myHeight; });

			sum /= myPoints.size();

			return sum;
		}

		inline float GetRainfall() const
		{
			float sum = std::accumulate(myPoints.begin(), myPoints.end(), 0.0f, [](float currentSum, Point* aPoint) {return currentSum + aPoint->myRainfall; });

			sum /= myPoints.size();

			return sum;
		}

		inline float GetTemperature() const
		{
			float sum = std::accumulate(myPoints.begin(), myPoints.end(), 0.0f, [](float currentSum, Point* aPoint) {return currentSum + aPoint->myTemperature; });

			sum /= myPoints.size();

			return sum;
		}
	};

	struct Grid
	{
		std::vector<Point> myPoints;
		std::vector<std::vector<Point*>> myRivers;
		std::vector<Cell> myCells;
	};
} // TerrainGeneration