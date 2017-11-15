#pragma once
#include <vector>
#include <numeric>

struct vec2
{
	vec2() :
		x(0.f),
		y(0.f)
	{}

	vec2(float aX, float aY) :
		x(aX),
		y(aY)
	{}

	float x;
	float y;

	void Normalize()
	{
		float norm = sqrt(pow(x, 2) + pow(y, 2));
		if (norm > 0.f)
		{
			x /= norm;
			y /= norm;
		}
	}
};

struct Point
{
	Point()
	{
		myPosition = vec2();
	}

	Point(const float x, const float y)
	{
		myPosition.x = x;
		myPosition.y = y;
		myNeighbours.reserve(8); // between 4 and 8 neighbours
		myFlags = 0;
		myRainfall = 0.f;
		myTemperature = 0.f;
	}

	vec2 myPosition;
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

	inline vec2 GetCenter() const
	{
		vec2 center = vec2(0.f, 0.f);
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

struct Edge
{
	Point* myA;
	Point* myB;
};
