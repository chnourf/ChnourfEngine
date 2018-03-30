#include "WorldGridGeneratorDebug.h"
#include "ppm.h"
#include "../Core/Vector.h"
#include <algorithm>
#include <future>
#include "../WorldGenerator/TerrainGenerationFunctions.h"

namespace Debug
{
	const unsigned int locPictureDimension{ 512 };// TerrainGeneration::GetMapTileAmount() };
	const unsigned int MetersPerPixel{ unsigned(TerrainGeneration::GetMapSize()) / locPictureDimension };
	const auto locWaterCol = vec3i(37, 125, 177);
	const auto locWaterColFloat = vec3f(locWaterCol.x / 255.f, locWaterCol.y / 255.f, locWaterCol.z / 255.f);

	vec2i PositionToPixel(vec2f aPostion)
	{
		unsigned int xPixel = (unsigned int)aPostion.x / MetersPerPixel;
		unsigned int yPixel = (unsigned int)aPostion.y / MetersPerPixel;
		if (yPixel == locPictureDimension)
		{
			yPixel -= 1u;
		}
		if (xPixel == locPictureDimension)
		{
			xPixel -= 1u;
		}

		return vec2i(xPixel, yPixel);
	}

	vec3f DeduceBiomeColor(TerrainGeneration::Biome aBiome)
	{
		switch (aBiome)
		{
		case TerrainGeneration::Biome::Snow:
			return vec3f(1.f, 1.f, 1.f);
			break;
		case TerrainGeneration::Biome::Tundra:
			return vec3f(221.f/255.f, 221.f / 255.f, 187.f / 255.f);
			break;
		case TerrainGeneration::Biome::Bare:
			return vec3f(187.f / 255.f, 187.f / 255.f, 187.f / 255.f);
			break;
		case TerrainGeneration::Biome::Scorched:
			return vec3f(153.f / 255.f, 153.f / 255.f, 153.f / 255.f);
			break;
		case TerrainGeneration::Biome::Taiga:
			return vec3f(204.f / 255.f, 212.f / 255.f, 187.f / 255.f);
			break;
		case TerrainGeneration::Biome::Shrubland:
			return vec3f(196.f / 255.f, 204.f / 255.f, 187.f / 255.f);
			break;
		case TerrainGeneration::Biome::TemperateDesert:
			return vec3f(208.f / 255.f, 252.f / 255.f, 182.f / 255.f);
			break;
		case TerrainGeneration::Biome::TemperateRainForest:
			return vec3f(159.f / 255.f, 1.f, 144.f / 255.f);
			break;
		case TerrainGeneration::Biome::TemperateDeciduousForest:
			return vec3f(160.f / 255.f, 181.f / 255.f, 149.f / 255.f);
			break;
		case TerrainGeneration::Biome::Grassland:
			return vec3f(162.f / 255.f, 179.f / 255.f, 104.f / 255.f);
			break;
		case TerrainGeneration::Biome::TropicalRainForest:
			return vec3f(162.f / 255.f, 229.f / 255.f, 96.f / 255.f);
			break;
		case TerrainGeneration::Biome::SubtropicalDesert:
			return vec3f(1.f, 187.f / 255.f, 28.f / 255.f);
			break;
		case TerrainGeneration::Biome::Sea:
			return locWaterColFloat;
			break;
		default:
			return vec3f(0.f);
			break;
		}
	}

	void DrawLine(ppm* anImage, const vec2i& aStart, const vec2i& aEnd, const vec3i& aCol)
	{
		unsigned int distance = Vector2<int>::Distance(aStart, aEnd);
		for (unsigned int i = 0; i <= distance; ++i)
		{
			float r = (float)i / (float)distance;
			auto point = Vector2<int>(glm::mix(aStart.x, aEnd.x, r), glm::mix(aStart.y, aEnd.y, r));

			anImage->setPixel(point, aCol);
		}
	}

	void DrawTriangle(ppm* anImage, const TerrainGeneration::Triangle& triangle, vec3i color)
	{
		auto& a = *triangle.myA;
		auto& b = *triangle.myB;
		auto& c = *triangle.myC;

		auto atob = Vector2<float>(b.myPosition.x - a.myPosition.x, b.myPosition.y - a.myPosition.y);
		auto btoc = Vector2<float>(c.myPosition.x - b.myPosition.x, c.myPosition.y - b.myPosition.y);
		auto ctoa = Vector2<float>(a.myPosition.x - c.myPosition.x, a.myPosition.y - c.myPosition.y);

		Vector2<float> maxBound;
		maxBound.x = std::max(a.myPosition.x, std::max(b.myPosition.x, c.myPosition.x));
		maxBound.y = std::max(a.myPosition.y, std::max(b.myPosition.y, c.myPosition.y));
		Vector2<float> minBound;
		minBound.x = std::min(a.myPosition.x, std::min(b.myPosition.x, c.myPosition.x));
		minBound.y = std::min(a.myPosition.y, std::min(b.myPosition.y, c.myPosition.y));

		for (float i = minBound.x; i < maxBound.x; i += (float)MetersPerPixel / 2.f)
		{
			for (float j = minBound.y; j < maxBound.y; j += (float)MetersPerPixel / 2.f)
			{
				Vector2<float> atoPixel = Vector2<float>(i - a.myPosition.x, j - a.myPosition.y);
				Vector2<float> btoPixel = Vector2<float>(i - b.myPosition.x, j - b.myPosition.y);
				Vector2<float> ctoPixel = Vector2<float>(i - c.myPosition.x, j - c.myPosition.y);
				bool isInTriangle = (Vector2<float>::Cross(atob, atoPixel) >= 0.f && Vector2<float>::Cross(btoc, btoPixel) >= 0.f && Vector2<float>::Cross(ctoa, ctoPixel) >= 0.f);

				if (isInTriangle)
				{
					anImage->setPixel(PositionToPixel(vec2f(i, j)), color);
				}
			}
		}
	}

	void DrawCellBiome(ppm* anImage, const TerrainGeneration::Cell& cell)
	{
		auto biomeCol = locWaterCol;

		if (cell.IsFlag(TerrainGeneration::PointTypeFlags::Land))
		{
			auto biomeColFloat = DeduceBiomeColor(cell.GetBiome());
			biomeCol = vec3i(biomeColFloat.x * 255, biomeColFloat.y * 255, biomeColFloat.z * 255);
		}

		const Vector2<float> center = cell.GetCenter();
		for (int i = 0; i < cell.myPoints.size(); ++i)
		{
			TerrainGeneration::Point point;
			point.myPosition = center;
			TerrainGeneration::Triangle triangle = TerrainGeneration::Triangle(cell.myPoints[i], cell.myPoints[(i + 1) % (cell.myPoints.size())], &point);
			DrawTriangle(anImage, triangle, biomeCol);
		}
	}

	void DrawCellRainfall(ppm* anImage, const TerrainGeneration::Cell& cell)
	{
		auto rainfallCol = vec3i(255 * cell.GetRainfall());

		if (!cell.IsFlag(TerrainGeneration::PointTypeFlags::Land))
		{
			rainfallCol = locWaterCol;
		}

		const auto center = cell.GetCenter();
		for (int i = 0; i < cell.myPoints.size(); ++i)
		{
			TerrainGeneration::Point point;
			point.myPosition = center;
			auto triangle = TerrainGeneration::Triangle(cell.myPoints[i], cell.myPoints[(i + 1) % (cell.myPoints.size())], &point);
			DrawTriangle(anImage, triangle, rainfallCol);
		}
	}

	void DrawCellTemperature(ppm* anImage, const TerrainGeneration::Cell& cell)
	{
		auto tempColor = vec3i(255 * cell.GetTemperature());

		const auto center = cell.GetCenter();
		for (int i = 0; i < cell.myPoints.size(); ++i)
		{
			TerrainGeneration::Point point;
			point.myPosition = center;
			auto triangle = TerrainGeneration::Triangle(cell.myPoints[i], cell.myPoints[(i + 1) % (cell.myPoints.size())], &point);
			DrawTriangle(anImage, triangle, tempColor);
		}
	}

	void DrawCellElevation(ppm* anImage, const TerrainGeneration::Cell& cell)
	{
		auto elevationCol = vec3i(255 * ( 0.3f + 0.5f*cell.GetElevation()/ TerrainGeneration::GetMultiplier()));

		if (!cell.IsFlag(TerrainGeneration::PointTypeFlags::Land))
		{
			elevationCol = locWaterCol;
		}

		if (cell.IsFlag(TerrainGeneration::PointTypeFlags::Mountain))
		{
			elevationCol.z = 0;
		}

		const auto center = cell.GetCenter();
		for (int i = 0; i < cell.myPoints.size(); ++i)
		{
			TerrainGeneration::Point point;
			point.myPosition = center;
			auto triangle = TerrainGeneration::Triangle(cell.myPoints[i], cell.myPoints[(i + 1) % (cell.myPoints.size())], &point);
			DrawTriangle(anImage, triangle, elevationCol);
		}
	}

	void DrawRiver(ppm* anImage, std::vector<TerrainGeneration::Point*> aRiver)
	{
		for (auto it = aRiver.begin(); it < aRiver.end() - 1; ++it)
		{
			DrawLine(anImage,
				PositionToPixel((*it)->myPosition),
				PositionToPixel((*(it + 1))->myPosition),
				locWaterCol);
		}
	}

	const int numThreads = 4;

	void DrawGrid(const TerrainGeneration::WorldGrid& aGrid)
	{
		// Create an empty PPM image
		auto biomeImage = new ppm(locPictureDimension, locPictureDimension);
		auto elevationImage = new ppm(locPictureDimension, locPictureDimension);
		auto rainfallImage = new ppm(locPictureDimension, locPictureDimension);
		auto temperatureImage = new ppm(locPictureDimension, locPictureDimension);

		std::vector<std::future<void>> handles;
		auto linesProcessedPerThread = locPictureDimension / numThreads;
		auto currentMin = 0;

		for (auto thread = 0; thread < numThreads; thread++)
		{
			handles.push_back(std::async(std::launch::async, [currentMin, linesProcessedPerThread, &aGrid, elevationImage, rainfallImage, biomeImage, temperatureImage]() {
				for (int i = currentMin; i < currentMin + linesProcessedPerThread; ++i)
				{
					for (int j = 0; j < locPictureDimension; ++j)
					{
						const float x = float(MetersPerPixel) * float(i - int(locPictureDimension) / 2);
						const float y = float(MetersPerPixel) * float(j - int(locPictureDimension) / 2);
						const auto elevation = TerrainGeneration::ComputeElevation(x, y, true);
						auto elevationCol = elevation < 0.f ? locWaterCol : vec3i(255 * 0.5f * (elevation / TerrainGeneration::GetMultiplier()));
						const auto temperature = TerrainGeneration::ComputeTemperature(x, elevation, y);
						const auto rainfall = aGrid.SampleGridRainfall(vec2f(x, y));
						auto tempCol = vec3i(200 * temperature, 0, 200 * (1.f - temperature));
						elevationImage->setPixel(vec2i(i, j), elevationCol);
						rainfallImage->setPixel(vec2i(i, j), vec3i(255 * rainfall));
						auto biomeCol = elevation < 0.f ? vec3f(locWaterCol.x / 255.f, locWaterCol.y / 255.f, locWaterCol.z / 255.f) : DeduceBiomeColor(TerrainGeneration::DeduceBiome(temperature, rainfall));
						biomeImage->setPixel(vec2i(i, j), vec3i(255 * biomeCol.x, 255 * biomeCol.y, 255 * biomeCol.z));
						temperatureImage->setPixel(vec2i(i, j), tempCol);
					}
				}
			}));

			currentMin += linesProcessedPerThread;
		}

		for (auto& handle : handles)
		{
			handle.get();
		}

		for (const auto& river : aGrid.GetRivers())
		{
			DrawRiver(biomeImage, river);
			DrawRiver(elevationImage, river);
		}

		biomeImage->write("Biomes.ppm");
		elevationImage->write("Elevation.ppm");
		rainfallImage->write("Rainfall.ppm");
		temperatureImage->write("Temperature.ppm");
		
		delete biomeImage;
		delete elevationImage;
		delete rainfallImage;
		delete temperatureImage;
	}

}