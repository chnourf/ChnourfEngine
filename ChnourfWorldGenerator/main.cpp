#include <iostream>
#include "PerlinNoise.h"
#include "ppm.h"
#include "math.h"
#include "Geometry.h"
#include <cassert>
#include <vector>
#include <numeric>
#include <algorithm>
#include <random>
#include <time.h>

# define M_PI           3.14159265358979323846  /* pi */
const unsigned int locPictureDimension = 2048u;
const unsigned int MetersPerPixel = 64;
const float locMapSize = (float)MetersPerPixel * (float)locPictureDimension;
const unsigned int locGridNumOfElements = 128;
const float locDistanceBetweenElements = locMapSize / (float)(locGridNumOfElements);
std::default_random_engine engine;
std::bernoulli_distribution boolDistribution;
std::uniform_real_distribution<float> floatDistribution(0.f, 1.f);
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

enum class flags
{
	Land = 1 << 0,
	Mountain = 1 << 1,
};

struct Grid
{
	std::vector<Point> points;
	std::vector<Cell> cells;
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
		else if (aRainfall > 0.15f)
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

void DrawTriangleBiome(ppm** anImage, const Cell& cell)
{
	float Elevation = cell.GetElevation();
	float Rainfall = cell.GetRainfall();
	float Temperature = cell.GetTemperature();
	std::vector<float> biomeCol;
	if (Elevation > locSeaLevel)
	{
		DeduceBiome(Temperature, Rainfall, biomeCol);
	}
	else
	{
		biomeCol.push_back(37.f);
		biomeCol.push_back(125.f);
		biomeCol.push_back(177.f);
	}

	const vec2 center = cell.GetCenter();
	for (int i = 0; i < cell.myPoints.size(); ++i)
	{
		Point point;
		point.myPosition = center;
		Triangle triangle = Triangle(cell.myPoints[i], cell.myPoints[(i + 1) % (cell.myPoints.size())], &point);
		DrawTriangle(anImage, triangle, biomeCol);
	}
}

void DrawTriangleRainfall(ppm** anImage, const Cell& cell)
{
	const float Rainfall = cell.GetRainfall();
	const float Elevation = cell.GetElevation();

	std::vector<float> elevationCol;
	for (int i = 0; i < 3; i++)
	{
		elevationCol.push_back(255.f * Rainfall);
	}

	if (Elevation < locSeaLevel)
	{
		for (int i = 0; i < 3; i++)
		{
			elevationCol[i] = 0.f;
		}
	}

	const vec2 center = cell.GetCenter();
	for (int i = 0; i < cell.myPoints.size(); ++i)
	{
		Point point;
		point.myPosition = center;
		Triangle triangle = Triangle(cell.myPoints[i], cell.myPoints[(i + 1) % (cell.myPoints.size())], &point);
		DrawTriangle(anImage, triangle, elevationCol);
	}
}

void DrawTriangleTemperature(ppm** anImage, const Cell& cell)
{
	const float Temperature = cell.GetTemperature();

	std::vector<float> elevationCol;
	for (int i = 0; i < 3; i++)
	{
		elevationCol.push_back(255.f * Temperature);
	}

	const vec2 center = cell.GetCenter();
	for (int i = 0; i < cell.myPoints.size(); ++i)
	{
		Point point;
		point.myPosition = center;
		Triangle triangle = Triangle(cell.myPoints[i], cell.myPoints[(i + 1) % (cell.myPoints.size())], &point);
		DrawTriangle(anImage, triangle, elevationCol);
	}
}

void DrawTriangleElevation(ppm** anImage, const Cell& cell)
{
	//bool isMountain = ((a.myFlags & (int)flags::Mountain) && (c.myFlags & (int)flags::Mountain) && (c.myFlags & (int)flags::Mountain));
	const float Elevation = cell.GetElevation();

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

	if (Elevation > locSeaLevel + 0.35f)
	{
		elevationCol[0] = 0.f;
		elevationCol[1] = 0.f;
	}

	const vec2 center = cell.GetCenter();
	for (int i = 0; i < cell.myPoints.size(); ++i)
	{
		Point point;
		point.myPosition = center;
		Triangle triangle = Triangle(cell.myPoints[i], cell.myPoints[(i + 1) % (cell.myPoints.size())], &point);
		DrawTriangle(anImage, triangle, elevationCol);
	}
}

void AddCell(Grid& aGrid, std::vector<Point*> points)
{
	const auto pointSize = points.size();
	for (int i = 0; i < pointSize; ++i)
	{
		auto& point = points[i];
		AddUnique(point->myNeighbours, points[(i + 1) % pointSize]);
		AddUnique(point->myNeighbours, points[(i + pointSize - 1) % pointSize]);
	}

	aGrid.cells.push_back(Cell(points));
}

void PropagateRainfall(Point* aPoint, vec2 aWindDirection, PerlinNoise pn)
{
	if (aPoint->myRainfall < 0.001f)
	{
		return;
	}

	// modifying wind vector to add some randomness
	vec2 newWindDirection;
	float seed = M_PI * (pn.noise(aPoint->myPosition.x * 5.f / locMapSize, aPoint->myPosition.y * 5.f / locMapSize, 0) - 0.5f); // between -90 and 90 degrees variation
	newWindDirection.x = cos(seed) * aWindDirection.x - sin(seed) * aWindDirection.y;
	newWindDirection.y = sin(seed) * aWindDirection.x + cos(seed) * aWindDirection.y;

	// calculating where the rainfall will go
	std::vector<float> rainfallTransmissions;
	for (auto& neighbour : aPoint->myNeighbours)
	{
		vec2 pointToNeighbour;
		pointToNeighbour.x = neighbour->myPosition.x - aPoint->myPosition.x;
		pointToNeighbour.y = neighbour->myPosition.y - aPoint->myPosition.y;
		pointToNeighbour.Normalize();

		float rainfallToTransfer = std::max(0.f, (pointToNeighbour.x * newWindDirection.x + pointToNeighbour.y * newWindDirection.y)) * 0.90f + 0.1f;
		rainfallTransmissions.push_back(rainfallToTransfer);
	}

	const float totalTransmission = std::accumulate(rainfallTransmissions.begin(), rainfallTransmissions.end(), 0.f);
	for (auto& transmission : rainfallTransmissions)
	{
		transmission /= totalTransmission;
	}

	// diffusing rainfall
	const float initialRainfall = aPoint->myRainfall;
	bool pointIsSea = (aPoint->myFlags & (int)flags::Land) == 0;
	for (int i = 0; i < aPoint->myNeighbours.size(); ++i)
	{
		auto& neighbour = aPoint->myNeighbours[i];

		auto landFlag = neighbour->myFlags & (int)flags::Land;
		bool neighbourIsSea = landFlag == 0;
		if (neighbourIsSea && pointIsSea)
		{
			continue;
		}

		vec2 pointToNeighbour;
		pointToNeighbour.x = neighbour->myPosition.x - aPoint->myPosition.x;
		pointToNeighbour.y = neighbour->myPosition.y - aPoint->myPosition.y;
		pointToNeighbour.Normalize();

		float rainfallDropped = initialRainfall * rainfallTransmissions[i];
		auto mountainFlag = neighbour->myFlags & (int)flags::Mountain;
		if (mountainFlag != 0)
		{
			// mountains influence rainfall, we symbolize it by reducing the quantity of rainfall transmitted by an arbitrary coefficient
			rainfallDropped *= 0.4f;
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

int main(int argc, char **argv)
{
	// Create an empty PPM image
	ppm* image = new ppm(locPictureDimension, locPictureDimension);

	unsigned int seed = (unsigned int)time(nullptr);
	engine.seed(seed);
	PerlinNoise pn(seed);

	Grid grid;
	const unsigned int horizontalAmountOfPoints = (2 * locGridNumOfElements + 1u);
	const unsigned int verticalAmountOfPoints = (locGridNumOfElements* 4.f / 3.f + 1u);
	grid.points.reserve(horizontalAmountOfPoints * verticalAmountOfPoints);
	const unsigned int verticalElements = verticalAmountOfPoints - 1u;
	grid.cells.reserve((locGridNumOfElements - 1u) * verticalElements);

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

			vec2 warp = vec2(floatDistribution(engine) - .5f, floatDistribution(engine) - .5f);
			float norm = sqrt(pow(warp.x, 2) + pow(warp.y, 2));
			if (norm > 1.f)
			{
				warp.x /= norm;
				warp.y /= norm;
			}

			if (i > 0 && i < verticalAmountOfPoints - 1)
			{
				adjustedY = clamp(y + warp.x * locDistanceBetweenElements / 2.f, 0.f, locMapSize);
			}
			if (j > 0 && j < horizontalAmountOfPoints - 1)
			{
				adjustedX = clamp(x + warp.y * locDistanceBetweenElements / 2.f, 0.f, locMapSize);
			}

			auto pointToAdd = Point(adjustedX, adjustedY);
			grid.points.push_back(pointToAdd);
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
			pointsToAdd.push_back(&grid.points[topLeftId]);
			pointsToAdd.push_back(&grid.points[topCenterId]);
			pointsToAdd.push_back(&grid.points[topRightId]);
			pointsToAdd.push_back(&grid.points[bottomRightId]);
			pointsToAdd.push_back(&grid.points[bottomCenterId]);
			pointsToAdd.push_back(&grid.points[bottomLeftId]);

			AddCell(grid, pointsToAdd);
		}
	}

	vec2 midPos = vec2(locMapSize / 2.f, locMapSize / 2.f);

	// ELEVATION
	std::cout << "generating height and mountains..." << std::endl;
	for (auto& point : grid.points)
	{
		const auto& pointPos = point.myPosition;

		float cellNoise = 0.f;

		float freq = 1.f;
		float amp = 1.f;


		// points far away from the center will "sink" allowing a border ocean. Distance is artificially modified with Perlin noise to create more irregularity
		float noiseDistAttenuation = 0.f;
		for (int d = 1; d <= 6; d++)
		{
			noiseDistAttenuation += pn.noise(3.f * pow(2, d) * pointPos.x / locMapSize, 3.f * pow(2, d) * pointPos.y / locMapSize, 0) / pow(2, d);
		}
		float distToCenter = (std::max(abs(pointPos.x - midPos.x), abs(pointPos.y - midPos.y))) * (1.f + noiseDistAttenuation);
		const float distAttenuation = clamp(10.f / locMapSize * (0.70f * locMapSize - abs(distToCenter)), 0.f, 1.0f);

		// warped fractal Perlin noise
		auto warpX = locMapSize / 6.f * (pn.noise(pointPos.x / (locMapSize / 4.f) + 0.3f, pointPos.y / (locMapSize / 4.f) + 0.3f, 0.f) - 0.5f);
		auto warpY = locMapSize / 6.f * (pn.noise(pointPos.x / (locMapSize / 4.f) + 0.3f, pointPos.y / (locMapSize / 4.f) + 0.3f, 1.f) - 0.5f);
		for (int d = 1; d <= 8; d++)
		{
			freq *= lacunarity;
			amp *= gain;

			auto softNoise = 0.f;

			softNoise = pn.noise(freq * (pointPos.x + warpX) / (locMapSize / 4.f), freq * (pointPos.y + warpY) / (locMapSize / 4.f), 0.f);

			cellNoise += softNoise*amp;
		}

		cellNoise *= distAttenuation;

		// MOUNTAINS
		const float locCoastalMountainsWidth = 0.04f;
		float coastalMountains = exp(-pow((cellNoise - locSeaLevel - locCoastalMountainsWidth / 2.f) / (locCoastalMountainsWidth), 2));
		float continentalMountains = 1.f / (1.f + exp(-100.f * (cellNoise - (locSeaLevel + 0.15f))));
		float someRandomNoise = 1.f / (1 + exp(-40.f * (pn.noise((pointPos.x + warpX) / (locMapSize / 4.f), (pointPos.y + warpY) / (locMapSize / 10.f), 0.f) - 0.6f)));
		cellNoise += (coastalMountains + continentalMountains) * someRandomNoise;

		point.myHeight = cellNoise;
		if (cellNoise > locSeaLevel)
		{
			point.myFlags |= (int)flags::Land;
		}
		if (cellNoise > locSeaLevel + 0.35f)
		{
			point.myFlags |= (int)flags::Mountain;
		}
	}

	// TEMPERATURE
	std::cout << "computing temperature..." << std::endl;
	for (auto& point : grid.points)
	{
		auto isSea = (point.myFlags & (int)flags::Land) == 0;
		point.myRainfall = isSea ? 1.f : 0.f;

		//point.myTemperature = 1 - abs(2.f / locMapSize * (point.myPosition.y - locMapSize / 2.f));
		point.myTemperature = sin(M_PI / locMapSize*(locMapSize - point.myPosition.y));
		float tempRandomness = 0.f;
		for (int d = 1; d <= 4; d++)
		{
			tempRandomness += pn.noise(5.f * pow(2, d) * point.myPosition.x / locMapSize, 5.f * pow(2, d) * point.myPosition.y / locMapSize, 0) / pow(2, d);
		}
		point.myTemperature = clamp(point.myTemperature + 0.5f * (tempRandomness - 0.5f), 0.f, 1.f);
		float altitudeInfluence = clamp((point.myHeight - locSeaLevel) * 0.5f, 0.f, 1.f);
		point.myTemperature = clamp(point.myTemperature - altitudeInfluence, 0.f, 1.f);
	}

	// RAINFALL
	std::cout << "diffusing rainfall..." << std::endl;
	const float phase = 2 * M_PI * floatDistribution(engine);
	vec2 windVector;
	windVector.x = sin(phase);
	windVector.y = cos(phase);
	// don't know yet how to link iterations number and resolution
	const int iterations = locGridNumOfElements / 2; //pow(locGridNumOfElements / 16, 2);
	for (int i = 0; i < iterations; ++i)
	{
		for (auto& point : grid.points)
		{
			PropagateRainfall(&point, windVector, pn);
		}
	}
	for (auto& point : grid.points)
	{
		auto isSea = (point.myFlags & (int)flags::Land) == 0;
		if (!isSea)
		{
			point.myRainfall = 5.f*point.myRainfall / (1.f + 5.f*point.myRainfall);
		}
	}
	for (auto& point : grid.points)
	{
		//point.myRainfall += (1.f - point.myPosition.x / locMapSize) * 0.3f;
		float rainRandomness = 0.f;
		auto warpX = locMapSize / 6.f * (pn.noise(point.myPosition.x / (locMapSize / 4.f) + 0.3f, point.myPosition.y / (locMapSize / 4.f) + 0.3f, 0.f) - 0.5f);
		auto warpY = locMapSize / 6.f * (pn.noise(point.myPosition.x / (locMapSize / 4.f) + 0.3f, point.myPosition.y / (locMapSize / 4.f) + 0.3f, 1.f) - 0.5f);
		for (int d = 1; d <= 4; d++)
		{
			rainRandomness += pn.noise(4.f * pow(2, d) * (point.myPosition.x + warpX) / locMapSize, 4.f * pow(2, d) * (point.myPosition.y + warpY) / locMapSize, 1) / pow(2, d);
		}
		point.myRainfall = clamp(point.myRainfall + (rainRandomness - 0.5f), 0.f, 1.f);
	}

	// DRAWING
	std::cout << "drawing output..." << std::endl;
	for (auto cell : grid.cells)
	{
		DrawTriangleBiome(&image, cell);
	}
	image->write("Biomes.ppm");

	for (auto cell : grid.cells)
	{
		DrawTriangleTemperature(&image, cell);
	}
	image->write("Temperature.ppm");

	for (auto cell : grid.cells)
	{
		DrawTriangleRainfall(&image, cell);
	}
	image->write("Rainfall.ppm");

	for (auto cell : grid.cells)
	{
		DrawTriangleElevation(&image, cell);
	}
	image->write("Elevation.ppm");

	return 0;
}