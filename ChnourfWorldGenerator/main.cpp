#include <iostream>
#include "PerlinNoise.h"
#include "ppm.h"
#include "Cell.h"
#include "math.h"
#include <vector>
#include <algorithm>
#include <random>
#include <time.h>

# define M_PI           3.14159265358979323846  /* pi */
const unsigned int locPictureDimension = 1024u;
const unsigned int MetersPerPixel = 64;
const float locMapSize = (float)MetersPerPixel * (float)locPictureDimension;
const unsigned int locGridNumOfElements = 256;
const float locDistanceBetweenElements = locMapSize / (float)(locGridNumOfElements - 1u);
std::default_random_engine engine;
std::bernoulli_distribution boolDistribution;
std::uniform_real_distribution<float> floatDistribution(-1.f, 1.f);
const unsigned int locDepth = 6u;
const float gain = 0.5f;
const float lacunarity = 1.90f;
const float locSeaLevel = 0.45f;

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

void CreateCol(std::vector<float>& anOutCol, float r, float g, float b)
{
	anOutCol.push_back(r);
	anOutCol.push_back(g);
	anOutCol.push_back(b);
}

void DeduceBiome(const float aTemperature, const float aRainfall, std::vector<float>& anOutCol)
{
	if (aTemperature < 0.25f)
	{
		if (aRainfall > 0.5f) // snow
		{
			CreateCol(anOutCol, 255.f, 255.f, 255.f);
		}
		else if (aRainfall > 0.35f) // tundra
		{
			CreateCol(anOutCol, 221.f, 221.f, 187.f);
		}
		else if (aRainfall > 0.15f) // bare
		{
			CreateCol(anOutCol, 187.f, 187.f, 187.f);
		}
		else // scorched
		{
			CreateCol(anOutCol, 153.f, 153.f, 153.f);
		}
	}
	else if (aTemperature < 0.5f)
	{
		if (aRainfall > 0.66f) // taiga
		{
			CreateCol(anOutCol, 204.f, 212.f, 187.f);
		}
		else if (aRainfall > 0.33f) // shrubland
		{
			CreateCol(anOutCol, 196.f, 204.f, 187.f);
		}
		else // temperate desert
		{
			CreateCol(anOutCol, 208.f, 252.f, 182.f);
		}
	}
	else if (aTemperature < 0.75)
	{
		if (aRainfall > 0.85f) // temperate rain forest
		{
			CreateCol(anOutCol, 144.f, 216.f, 148.f);
		}
		else if (aRainfall > 0.5f) // temperate deciduous forest
		{
			CreateCol(anOutCol, 160.f, 181.f, 149.f);
		}
		else if (aRainfall > 0.15f)
		{
			CreateCol(anOutCol, 162.f, 179.f, 104.f); // grassland
		}
		else
		{
			CreateCol(anOutCol, 208.f, 255.f, 182.f); // temperate desert
		}
	}
	else
	{
		if (aRainfall > 0.65f)
		{
			CreateCol(anOutCol, 136.f, 207.f, 149.f); // tropical rain forest
		}
		else if (aRainfall > 0.35f)
		{
			CreateCol(anOutCol, 159.f, 234.f, 144.f); // temperate rain forest
		}
		else if (aRainfall > 0.10f)
		{
			CreateCol(anOutCol, 162.f, 179.f, 104.f); // grassland
		}
		else
		{
			CreateCol(anOutCol, 255.f, 187.f, 28.f); // subtropical desert
		}
	}
}

void DrawTriangle(ppm** anImage, const Triangle& triangle, std::vector<float> color)
{
	auto& a = *triangle.myA;
	auto& b = *triangle.myB;
	auto& c = *triangle.myC;

	vec2 atob = vec2(b.myPosition.x - a.myPosition.x, b.myPosition.y - a.myPosition.y);
	vec2 btoc = vec2(c.myPosition.x - b.myPosition.x, c.myPosition.y - b.myPosition.y);
	vec2 ctoa = vec2(a.myPosition.x - c.myPosition.x, a.myPosition.y - c.myPosition.y);

	vec2 maxBound;
	maxBound.x = std::max(a.myPosition.x, std::max(b.myPosition.x, c.myPosition.x));
	maxBound.y = std::max(a.myPosition.y, std::max(b.myPosition.y, c.myPosition.y));
	vec2 minBound;
	minBound.x = std::min(a.myPosition.x, std::min(b.myPosition.x, c.myPosition.x));
	minBound.y = std::min(a.myPosition.y, std::min(b.myPosition.y, c.myPosition.y));

	for (float i = minBound.x; i < maxBound.x; i += (float)MetersPerPixel / 2.f)
	{
		for (float j = minBound.y; j < maxBound.y; j += (float)MetersPerPixel / 2.f)
		{
			vec2 atoPixel = vec2(i - a.myPosition.x, j - a.myPosition.y);
			vec2 btoPixel = vec2(i - b.myPosition.x, j - b.myPosition.y);
			vec2 ctoPixel = vec2(i - c.myPosition.x, j - c.myPosition.y);
			bool isInTriangle = (cross(atob, atoPixel) >= 0.f && cross(btoc, btoPixel) >= 0.f && cross(ctoa, ctoPixel) >= 0.f);

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

				(*anImage)->r[xPixel + yPixel * locPictureDimension] = color[0];
				(*anImage)->g[xPixel + yPixel * locPictureDimension] = color[1];
				(*anImage)->b[xPixel + yPixel * locPictureDimension] = color[2];
			}
		}
	}
}

void DrawTriangleBiome(ppm** anImage, const Triangle& triangle)
{
	auto& a = *triangle.myA;
	auto& b = *triangle.myB;
	auto& c = *triangle.myC;
	bool isLand = ((a.myFlags & (int)flags::Land) && (c.myFlags & (int)flags::Land) && (c.myFlags & (int)flags::Land));
	float Rainfall = (a.myRainfall + b.myRainfall + c.myRainfall) / 3.f;
	float Temperature = (a.myTemperature + b.myTemperature + c.myTemperature) / 3.f;
	std::vector<float> biomeCol;
	if (isLand)
	{
		DeduceBiome(Temperature, Rainfall, biomeCol);
	}
	else
	{
		biomeCol.push_back(37.f);
		biomeCol.push_back(125.f);
		biomeCol.push_back(177.f);
	}

	DrawTriangle(anImage, triangle, biomeCol);
}

void DrawTriangleRainfall(ppm** anImage, const Triangle& triangle)
{
	auto& a = *triangle.myA;
	auto& b = *triangle.myB;
	auto& c = *triangle.myC;
	float Rainfall = (a.myRainfall + b.myRainfall + c.myRainfall) / 3.f;

	std::vector<float> rainfallCol;
	for (int i = 0; i < 3; i++)
	{
		rainfallCol.push_back(255.f * Rainfall);
	}
	DrawTriangle(anImage, triangle, rainfallCol);
}

void DrawTriangleTemperature(ppm** anImage, const Triangle& triangle)
{
	auto& a = *triangle.myA;
	auto& b = *triangle.myB;
	auto& c = *triangle.myC;
	float Temperature = (a.myTemperature + b.myTemperature + c.myTemperature) / 3.f;

	std::vector<float> tempCol;
	for (int i = 0; i < 3; i++)
	{
		tempCol.push_back(255.f * Temperature);
	}
	DrawTriangle(anImage, triangle, tempCol);
}

void DrawTriangleElevation(ppm** anImage, const Triangle& triangle)
{
	auto& a = *triangle.myA;
	auto& b = *triangle.myB;
	auto& c = *triangle.myC;
	bool isMountain = ((a.myFlags & (int)flags::Mountain) && (c.myFlags & (int)flags::Mountain) && (c.myFlags & (int)flags::Mountain));
	float Elevation = (a.myHeight + b.myHeight + c.myHeight) / 3.f;

	std::vector<float> elevationCol;
	for (int i = 0; i < 3; i++)
	{
		elevationCol.push_back(255.f * clamp(Elevation / 2.f, 0.f, 1.f));
	}

	if (Elevation < locSeaLevel)
	{
		for (int i = 0; i < 3; i++)
		{
			elevationCol[i] = 0.f;
		}
	}

	if (isMountain)
	{
		elevationCol[0] = 0.f;
		elevationCol[1] = 0.f;
	}

	DrawTriangle(anImage, triangle, elevationCol);
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
	float seed = aPoint->myPosition.x + aPoint->myPosition.y;
	seed -= locMapSize / 2.f;
	seed /= 0.5f*locMapSize;

	vec2 newWindDirection;
	newWindDirection.x = cos(seed);// *aWindDirection.x - sin(seed) * aWindDirection.y;
	newWindDirection.y = sin(seed);// *aWindDirection.x + cos(seed) * aWindDirection.y;


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
		float rainfallToTransfer = pointToNeighbour.x * newWindDirection.x + pointToNeighbour.y * newWindDirection.y;
		if (rainfallToTransfer > 0.f)
		{
			auto mountainFlag = neighbour->myFlags & (int)flags::Mountain;
			float coeff = 0.1f;
			float rainfallDropped = coeff * aPoint->myRainfall * rainfallToTransfer;
			if (mountainFlag != 0)
			{
				rainfallDropped *= 0.3f;
			}
			bool pointIsSea = (aPoint->myFlags & (int)flags::Land) == 0;
			if (!pointIsSea)
			{
				aPoint->myRainfall = std::max(0.f, aPoint->myRainfall - rainfallDropped);
			}
			neighbour->myRainfall = std::min(1.f, neighbour->myRainfall + rainfallDropped);
		}
	}
}

int main(int argc, char **argv)
{
	// Create an empty PPM image
	ppm* image = new ppm(locPictureDimension, locPictureDimension);

	unsigned int seed = 100;//(unsigned int)time(nullptr);

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

		auto warpX = locMapSize / 6.f * (pn.noise(pointPos.x / (locMapSize / 4.f) + 0.3f, pointPos.y / (locMapSize / 4.f) + 0.3f, 0.f) - 0.5f);
		auto warpY = locMapSize / 6.f * (pn.noise(pointPos.x / (locMapSize / 4.f) + 0.3f, pointPos.y / (locMapSize / 4.f) + 0.3f, 1.f) - 0.5f);

		float noiseDistAttenuation = 0.f;
		for (int d = 1; d <= 4; d++)
		{
			noiseDistAttenuation += pn.noise(5.f * pow(2,d) * pointPos.x / locMapSize, 5.f * pow(2, d) * pointPos.y / locMapSize, 0) / pow(2, d);
		}

		float distToCenter = (std::max(abs(pointPos.x - midPos.x), abs(pointPos.y - midPos.y))) * (1.f + noiseDistAttenuation);
		const float distAttenuation = clamp(10.f / locMapSize * (0.70f * locMapSize - abs(distToCenter)), 0.f, 1.0f);

		for (int d = 1; d <= 8; d++)
		{
			freq *= lacunarity;
			amp *= gain;

			auto softNoise = 0.f;

			softNoise = pn.noise(freq * (pointPos.x + warpX) / (locMapSize / 4.f), freq * (pointPos.y + warpY) / (locMapSize / 4.f), 0.f) * distAttenuation;

			cellNoise += softNoise*amp;
		}

		const float locCoastalMountainsWidth = 0.05f;
		float coastalMountains = exp(-pow((cellNoise - locSeaLevel - locCoastalMountainsWidth / 2.f) / (locCoastalMountainsWidth), 2));

		float continentalMountains = 1.f / (1.f + exp(-100.f * (cellNoise - (locSeaLevel+ 0.15f))));

		float someRandomNoise = 1.f / (1 + exp(-40.f * (pn.noise((pointPos.x + warpX) / (locMapSize / 4.f), (pointPos.y + warpY) / (locMapSize / 10.f), 0.f) - 0.6f)));

		cellNoise += (coastalMountains + continentalMountains) * someRandomNoise;

		point.myHeight = cellNoise;

		if (cellNoise > locSeaLevel)
		{
			point.myFlags |= (int)flags::Land;
		}
		if (cellNoise > .8f)
		{
			point.myFlags |= (int)flags::Mountain;
		}
	}

	//const float phase = 180.f * floatDistribution(engine);
	vec2 windVector;
	//windVector.x = sin(phase);
	//windVector.y = cos(phase);
	windVector.x = 1.f;
	windVector.y = 1.f;
	windVector.Normalize();

	for (auto& point : grid.points)
	{
		auto isSea = (point.myFlags & (int)flags::Land) == 0;
		point.myRainfall = isSea ? 1.f : 0.f;

		//point.myTemperature = 1 - abs(2.f / locMapSize * (point.myPosition.y - locMapSize / 2.f));
		point.myTemperature = sin(M_PI / locMapSize*(locMapSize - point.myPosition.y));
		point.myTemperature = clamp(point.myTemperature + 0.2f * (pn.noise(point.myPosition.x / (locMapSize * 0.1f), point.myPosition.y / (locMapSize * 0.1f), 0.f) - 0.5f), 0.f, 1.f);
		float altitudeInfluence = clamp((point.myHeight - locSeaLevel) * 0.5f, 0.f, 1.f);
		point.myTemperature = clamp(point.myTemperature - altitudeInfluence, 0.f, 1.f);
	}

	for (int i = 0; i < locGridNumOfElements * 2; ++i)
	{
		for (auto& point : grid.points)
		{
			PropagateRainfall(&point, windVector);
		}
	}

	for (auto triangle : grid.triangles)
	{
		DrawTriangleBiome(&image, triangle);
	}

	// Save the image in a binary PPM file
	image->write("Biomes.ppm");

	for (auto triangle : grid.triangles)
	{
		DrawTriangleTemperature(&image, triangle);
	}
	image->write("Temperature.ppm");

	for (auto triangle : grid.triangles)
	{
		DrawTriangleRainfall(&image, triangle);
	}
	image->write("Rainfall.ppm");

	for (auto triangle : grid.triangles)
	{
		DrawTriangleElevation(&image, triangle);
	}
	image->write("Elevation.ppm");

	return 0;
}