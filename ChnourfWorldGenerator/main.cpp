#include <iostream>
#include "PerlinNoise.h"
#include "ppm.h"
#include "Cell.h"
#include "math.h"
#include <vector>
#include <algorithm>
#include <random>
#include <time.h>

const unsigned int locPictureDimension = 2048u;
const unsigned int MetersPerPixel = 30;
const float locMapSize = (float)MetersPerPixel * (float)locPictureDimension;
const unsigned int locGridNumOfElements = 256;
const float locDistanceBetweenElements = locMapSize / (float)(locGridNumOfElements - 1u);
std::default_random_engine engine;
std::bernoulli_distribution boolDistribution;
std::uniform_real_distribution<float> floatDistribution(-1.f, 1.f);
const unsigned int locDepth = 6u;
const float gain = 0.5f;
const float lacunarity = 1.90f;

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

struct vec2
{
	vec2():
		x(0.f),
		y(0.f)
	{}

	vec2(float aX, float aY):
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

enum class flags
{
	Land = 1 << 0,
	Mountain = 1 << 1,
};

struct Point
{
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

	Triangle* myNeighbours[3];
};

struct Grid
{
	std::vector<Point> points;
	std::vector<Triangle> triangles;
};

void AddUnique(std::vector<Point*>& aVector, Point* anObject)
{
	if (std::find(aVector.begin(), aVector.end(), anObject) == aVector.end()) {
		aVector.push_back(anObject);
	}
}

float lerp(float t, float a, float b) {
	return a + t * (b - a);
}

float cross(vec2 aX, vec2 aY)
{
	return aX.x * aY.y - aY.x * aX.y;
}

void DrawTriangle(ppm** anImage, const Triangle& triangle)
{
	auto& a = *triangle.myA;
	auto& b = *triangle.myB;
	auto& c = *triangle.myC;
	bool isLand = ((a.myFlags & (int)flags::Land) && (c.myFlags & (int)flags::Land) && (c.myFlags & (int)flags::Land));
	bool isMountain = ((a.myFlags & (int)flags::Mountain) && (c.myFlags & (int)flags::Mountain) && (c.myFlags & (int)flags::Mountain));
	float Rainfall = (a.myRainfall + b.myRainfall + c.myRainfall) / 3.f;

	vec2 atob = vec2(b.myPosition.x - a.myPosition.x, b.myPosition.y - a.myPosition.y);
	vec2 btoc = vec2(c.myPosition.x - b.myPosition.x, c.myPosition.y - b.myPosition.y);
	vec2 ctoa = vec2(a.myPosition.x - c.myPosition.x, a.myPosition.y - c.myPosition.y);

	vec2 maxBound;
	maxBound.x = std::max(a.myPosition.x, std::max(b.myPosition.x, c.myPosition.x));
	maxBound.y = std::max(a.myPosition.y, std::max(b.myPosition.y, c.myPosition.y));
	vec2 minBound;
	minBound.x = std::min(a.myPosition.x, std::min(b.myPosition.x, c.myPosition.x));
	minBound.y = std::min(a.myPosition.y, std::min( b.myPosition.y, c.myPosition.y));

	for (float i = minBound.x; i < maxBound.x; i+= (float)MetersPerPixel)
	{
		for (float j = minBound.y; j < maxBound.y; j+= (float)MetersPerPixel)
		{
			vec2 atoPixel = vec2(i - a.myPosition.x, j - a.myPosition.y);
			vec2 btoPixel = vec2(i - b.myPosition.x, j - b.myPosition.y);
			vec2 ctoPixel = vec2(i - c.myPosition.x, j - c.myPosition.y);
			bool isInTriangle = (cross(atob, atoPixel) > 0.f && cross(btoc, btoPixel) > 0.f && cross(ctoa, ctoPixel) > 0.f);

			if (isInTriangle)
			{
				unsigned int xPixel = (unsigned int)i / MetersPerPixel;
				unsigned int yPixel = (unsigned int)j / MetersPerPixel;
				if (yPixel == locPictureDimension)
				{
					yPixel -= 1u;
				}
				if (xPixel == locPictureDimension)
				{
					xPixel -= 1u;
				}

				(*anImage)->r[xPixel + yPixel * locPictureDimension] = 255.f * Rainfall;//isMountain ? 255.f : 0.f;
				(*anImage)->g[xPixel + yPixel * locPictureDimension] = 255.f * Rainfall;//isLand ? 255.f : 0.f;
				(*anImage)->b[xPixel + yPixel * locPictureDimension] = 255.f * Rainfall;//isLand ? 0.f : 255.f;
			}
		}
	}
}

void DrawLine(ppm** anImage, vec2 aStart, vec2 aEnd, bool aIsSea)
{
	const float dist = sqrt(pow(aStart.x - aEnd.x, 2) + pow(aStart.y - aEnd.y, 2));
	const unsigned int distInPixels = std::max(0.f, dist / MetersPerPixel - 1u);
	for (int i = 1; i < distInPixels; ++i)
	{
		const float lerpPosX = lerp((float)i / (float)distInPixels, aStart.x, aEnd.x);
		const float lerpPosY = lerp((float)i / (float)distInPixels, aStart.y, aEnd.y);

		unsigned int xPixel = (unsigned int)lerpPosX / MetersPerPixel;
		unsigned int yPixel = (unsigned int)lerpPosY / MetersPerPixel;
		if (yPixel == locPictureDimension)
		{
			yPixel -= 1u;
		}
		if (xPixel == locPictureDimension)
		{
			xPixel -= 1u;
		}

		(*anImage)->r[xPixel + yPixel * locPictureDimension] = 0.f;
		(*anImage)->g[xPixel + yPixel * locPictureDimension] = 0.f;
		(*anImage)->b[xPixel + yPixel * locPictureDimension] = aIsSea ? 255.f : 0.f;
	}
}

void AddTriangle(const unsigned int aFirstIndex, const unsigned int aSecondIndex, const unsigned int aThirdIndex, Grid& aGrid)
{
	auto& pointA = aGrid.points[aFirstIndex];
	auto& pointB = aGrid.points[aSecondIndex];
	auto& pointC = aGrid.points[aThirdIndex];

	AddUnique(pointA.myNeighbours, &pointB);
	AddUnique(pointB.myNeighbours, &pointA);

	AddUnique(pointA.myNeighbours, &pointC);
	AddUnique(pointC.myNeighbours, &pointA);

	AddUnique(pointB.myNeighbours, &pointC);
	AddUnique(pointC.myNeighbours, &pointB);

	aGrid.triangles.push_back(Triangle(&pointA, &pointB, &pointC));
}

void PropagateRainfall(Point* aPoint, vec2 aWindDirection)
{
	for (auto& neighbour : aPoint->myNeighbours)
	{
		auto landFlag = neighbour->myFlags & (int)flags::Land;
		bool neighbourIsSea = landFlag == 0;
		if (neighbourIsSea)
		{
			continue;
		}

		vec2 pointToNeighbour;
		pointToNeighbour.x = neighbour->myPosition.x - aPoint->myPosition.x;
		pointToNeighbour.y = neighbour->myPosition.y - aPoint->myPosition.y;
		pointToNeighbour.Normalize();
		float rainfallToTransfer = pointToNeighbour.x * aWindDirection.x + pointToNeighbour.y * aWindDirection.y;
		if (rainfallToTransfer > 0.f)
		{
			auto mountainFlag = neighbour->myFlags & (int)flags::Mountain;
			float coeff = 0.05f;
			float rainfallDropped = coeff * aPoint->myRainfall * rainfallToTransfer;
			bool pointIsSea = (aPoint->myFlags & (int)flags::Land) == 0;
			if (!pointIsSea)
			{
				aPoint->myRainfall = std::max(0.f, aPoint->myRainfall - rainfallDropped);
			}
			if (mountainFlag != 0)
			{
				rainfallDropped *= 0.9f;
			}
				neighbour->myRainfall = std::min(1.f, neighbour->myRainfall + rainfallDropped);
		}
	}
}

int main(int argc, char **argv)
{
	// Create an empty PPM image
	ppm* image = new ppm(locPictureDimension, locPictureDimension);

	unsigned int seed = 0;// (unsigned int)time(nullptr);

	engine.seed(seed);
	PerlinNoise pn(seed);

	Grid grid;
	grid.points.reserve(locGridNumOfElements * locGridNumOfElements);
	grid.triangles.reserve((locGridNumOfElements - 1u) * (locGridNumOfElements - 1u) * 2u);

	for (unsigned int i = 0; i < locGridNumOfElements; ++i)
	{
		for (unsigned int j = 0; j < locGridNumOfElements; ++j)
		{
			const auto x = (float)j * locDistanceBetweenElements;
			const auto y = (float)i * locDistanceBetweenElements;

			auto adjustedX = x; 
			auto adjustedY = y;

			vec2 warp;
			warp.x = floatDistribution(engine);
			warp.y = floatDistribution(engine);
			float norm = sqrt(pow(warp.x, 2) + pow(warp.y, 2));
			if (norm > 1.f)
			{
				warp.x /= norm;
				warp.y /= norm;
			}

			if (i > 0 && i < locGridNumOfElements - 1)
			{
				adjustedY = clamp(y + warp.x * locDistanceBetweenElements / 3.f, 0.f, locMapSize);
			}
			if (j > 0 && j < locGridNumOfElements - 1)
			{
				adjustedX = clamp(x + warp.y * locDistanceBetweenElements / 3.f, 0.f, locMapSize);
			}

			auto pointToAdd = Point(adjustedX, adjustedY);
			pointToAdd.myTemperature = adjustedY / locMapSize;
			grid.points.push_back(pointToAdd);
			
			unsigned int xPixel = (unsigned int)(adjustedX / MetersPerPixel);
			unsigned int yPixel = (unsigned int)(adjustedY / MetersPerPixel);
			if (yPixel == locPictureDimension)
			{
				yPixel -= 1u;
			}
			if (xPixel == locPictureDimension)
			{
				xPixel -= 1u;
			}

			image->r[xPixel + yPixel * locPictureDimension] = 255.f;
			image->g[xPixel + yPixel * locPictureDimension] = 0.f;
			image->b[xPixel + yPixel * locPictureDimension] = 0.f;
		}
	}

	for (unsigned int i = 0; i < locGridNumOfElements - 1; ++i)
	{
		for (unsigned int j = 0; j < locGridNumOfElements - 1; ++j)
		{
			const auto topLeftId = j + i * locGridNumOfElements;
			const auto bottomLeftId = j + (i + 1) * locGridNumOfElements;
			const auto bottomRightId = (j + 1) + (i + 1) * locGridNumOfElements;
			const auto topRightId = (j + 1) + i * locGridNumOfElements;

			if (boolDistribution(engine))
			{
				AddTriangle(topLeftId, topRightId, bottomLeftId, grid);
				AddTriangle(topRightId, bottomRightId, bottomLeftId, grid);
			}
			else
			{
				AddTriangle(topLeftId, topRightId, bottomRightId, grid);
				AddTriangle(topLeftId, bottomRightId, bottomLeftId, grid);
			}
		}
	}

	vec2 midPos;
	midPos.x = locMapSize / 2.f;
	midPos.y = locMapSize / 2.f;

	for (auto& point : grid.points)
	{
		const auto& pointPos = point.myPosition;

		float cellNoise = 0;

		float freq = 1.f;
		float amp = 1.f;

		auto warpX = locMapSize/4.f * pn.noise(pointPos.x / (locMapSize/4.f) + 0.3f, pointPos.y / (locMapSize / 4.f) + 0.3f, 0.f) - 0.5f;
		auto warpY = locMapSize/4.f * pn.noise(pointPos.x / (locMapSize / 4.f) + 0.3f, pointPos.y / (locMapSize / 4.f) + 0.3f, 1.f) - 0.5f;

		float distToCenter = (std::max(abs(pointPos.x - midPos.x), abs(pointPos.y - midPos.y))) * (1.f + pn.noise(5.f * pointPos.x / locMapSize, 5.f * pointPos.y / locMapSize, 0));
		const float distAttenuation = clamp(10.f / locMapSize * (0.70f * locMapSize - abs(distToCenter)), 0.f, 1.0f);

		for (int d = 1; d <= 8; d++)
		{
			freq *= lacunarity;
			amp *= gain;

			auto softNoise = 0.f;

			softNoise = pn.noise(freq * (pointPos.x + warpX) / (locMapSize / 4.f), freq * (pointPos.y + warpY) / (locMapSize / 4.f), 0.f) * distAttenuation;

			cellNoise += softNoise*amp;
		}

		if ((cellNoise > 0.45f && cellNoise < 0.52f) || cellNoise > 0.6f)
		{
			cellNoise += 1.5f * pow(pn.noise((pointPos.x + warpX) / (locMapSize / 4.f), (pointPos.y + warpY) / (locMapSize / 10.f), 0.f), 2);
		}

		if (cellNoise > 0.45f)
		{
			point.myFlags |= (int)flags::Land;
		}
		if (cellNoise > 1.f)
		{
			point.myFlags |= (int)flags::Mountain;
		}
	}

	vec2 windVector;
	windVector.x = 1.f;
	windVector.y = 1.f;
	windVector.Normalize();

	for (auto& point : grid.points)
	{
		auto isSea = (point.myFlags & (int)flags::Land) == 0;
		if (isSea)
		{
			point.myRainfall = 1.f;
		}
	}

	for (int i = 0; i < 500; ++i)
	{
		for (auto& point : grid.points)
		{
			PropagateRainfall(&point, windVector);
		}
	}


	for (auto triangle : grid.triangles)
	{
		DrawTriangle(&image, triangle);
	}

	// Save the image in a binary PPM file
	image->write("Elevation.ppm");

	return 0;
}