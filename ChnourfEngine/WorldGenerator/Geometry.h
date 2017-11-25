#pragma once

#include <vector>
#include <numeric>
#include "../Core/Vector.h"
#include "../WorldGenerator/TerrainGenerationFunctions.h"
#include "../Core/Intersection.h"

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
			myPoints(aPoints),
			myBiome(Biome::Invalid)
		{
			vec2f minPoint = vec2f(std::numeric_limits<float>::max());
			vec2f maxPoint = vec2f(-std::numeric_limits<float>::min());
			for (auto point : myPoints)
			{
				minPoint.x = std::min(minPoint.x, point->myPosition.x);
				minPoint.y = std::min(minPoint.y, point->myPosition.y);
				maxPoint.x = std::max(maxPoint.x, point->myPosition.x);
				maxPoint.y = std::max(maxPoint.y, point->myPosition.y);
			}

			myAABB = AABB(vec3f(minPoint.x, 0.f, minPoint.y), vec3f(maxPoint.x, 0.f, maxPoint.y));
		}

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

		inline bool IsFlag(PointTypeFlags aFlag) const
		{
			auto result = true;
			for (auto point : myPoints)
			{
				result &= ((point->myFlags & static_cast<int>(aFlag)) != 0);
			}

			return result;
		}

		inline void SetBiome()
		{
			assert(myBiome == Biome::Invalid);
			myBiome = IsFlag(PointTypeFlags::Land) ? DeduceBiome(GetTemperature(), GetRainfall()) : Biome::Sea;
		}

		inline Biome GetBiome() const
		{
			//assert(myBiome != Biome::Invalid);
			return myBiome;
		}
		AABB myAABB;

	private:
		Biome myBiome;
	};

	struct Grid
	{
		std::vector<Point> myPoints;
		std::vector<std::vector<Point*>> myRivers;
		std::vector<Cell> myCells;
	};
} // TerrainGeneration