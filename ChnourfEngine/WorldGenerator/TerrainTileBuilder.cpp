#include "TerrainTileBuilder.h"
#include"TerrainTile.h"
#include"TerrainGenerationFunctions.h"
#include <time.h>
#include <iostream>
#include "../Core/Vector.h"
#include "../WorldGenerator/TerrainManager.h"

const float scale = .5f;
const float locMultiplier = 50.f/scale; // noise result between 0 and this value (in meters)
const float gain = 0.5f;
const float lacunarity = 1.90f;

TerrainTileBuildingTask::TerrainTileBuildingTask(const int aSeed, const unsigned int aTileSize, float aTileResolution, TerrainTile* anEmptyTile):
	myTileSize(aTileSize),
	myNoiseDepth(8),
	myTileResolution(aTileResolution),
	myWorldCell(anEmptyTile->myWorldCell)
{
	myPerlin = PerlinNoise(aSeed);
	myRandomEngine.seed(aSeed);
	myHandle = std::async(std::launch::async, [this, anEmptyTile]() {BuildTile(anEmptyTile);});
}

void TerrainTileBuildingTask::BuildTile(TerrainTile* aTile)
{
	if (!aTile)
	{
		std::cout << "ERROR : tile given to build does not exist" << std::endl;
		return;
	}

	float minHeight = std::numeric_limits<float>::max();
	float maxHeight = std::numeric_limits<float>::min();

	std::vector<TerrainElement> temporaryElements;
	temporaryElements.reserve(myTileSize * myTileSize);

	//computing elevation
	for (unsigned int i = 0; i < myTileSize; ++i) {     // y
		for (unsigned int j = 0; j < myTileSize; ++j) {  // x
			const float x = ((float)j / ((float)myTileSize-1) + aTile->GetGridIndex().y) * myTileSize * myTileResolution;
			const float y = ((float)i / ((float)myTileSize-1) + aTile->GetGridIndex().x) * myTileSize * myTileResolution;

			const auto tileNoise = SamplePerlinNoise(x, y);

			if (tileNoise < minHeight)
			{
				minHeight = tileNoise;
			}
			if (tileNoise > maxHeight)
			{
				maxHeight = tileNoise;
			}

			temporaryElements.push_back(TerrainElement(tileNoise, glm::vec3()));
		}
	}

	aTile->SetMinHeight(minHeight);
	aTile->SetMaxHeight(maxHeight);

	//std::vector<TerrainElement> elementsBeforeErosion = temporaryElements;

	////computing erosion, could be moved to presets.txt
	//TerrainGeneration::ErosionParams params;
	//params.Kq = 1.5f;
	//params.Kevap = 0.1f;
	//params.Kerosion = .9f;
	//params.Kdepos = .02f;
	//params.Ki = .01f;
	//params.minSlope = 0.05f;
	//params.g = 1.f;
	//TerrainGeneration::ComputeErosion(temporaryElements, 10000, params, myTileSize, myRandomEngine);

	//// lerping the edges of the tiles to ensure continuity
	//for (unsigned int i = 0; i < myTileSize; ++i) {
	//	for (unsigned int j = 0; j < myTileSize; ++j) {

	//		auto index = i + j * myTileSize;
	//		auto lerpFactor = glm::clamp(5.5f - abs(12.f * (float)i / (float)myTileSize - 6.f), 0.f, 1.f);
	//		lerpFactor *= glm::clamp(5.5f - abs(12.f * (float)j / (float)myTileSize - 6.f), 0.f, 1.f);
	//		auto& el = temporaryElements[index].myElevation;
	//		auto& bel = elementsBeforeErosion[index].myElevation;
	//		el = bel + lerpFactor * (el - bel);
	//	}
	//}

	//computing normals based on elevation
	for (unsigned int i = 0; i < myTileSize; ++i) {     // y
		for (unsigned int j = 0; j < myTileSize; ++j) {  // x

			auto delta = 1.f / ((float)myTileSize - 1);
			const float x = ((float)j * delta + aTile->GetGridIndex().y) * myTileSize * myTileResolution;
			const float y = ((float)i * delta + aTile->GetGridIndex().x) * myTileSize * myTileResolution;

			const auto index = j + i * myTileSize;

			// technically erosion should have changed the elevation of the edges but we suppose it's negligible
			const auto s01 = (j == 0) ? SamplePerlinNoise(x - delta * myTileSize * myTileResolution, y) : temporaryElements[index - 1].myElevation;
			const auto s21 = (j == myTileSize - 1) ? SamplePerlinNoise(x + delta * myTileSize * myTileResolution, y) : temporaryElements[index + 1].myElevation;
			const auto s10 = (i == 0) ? SamplePerlinNoise(x, y - delta * myTileSize * myTileResolution) : temporaryElements[index - myTileSize].myElevation;
			const auto s12 = (i == myTileSize - 1) ? SamplePerlinNoise(x, y + delta * myTileSize * myTileResolution) : temporaryElements[index + myTileSize].myElevation;
			const glm::vec3 va = glm::normalize(glm::vec3(2 * delta, (s21 - s01) / (myTileSize * myTileResolution), 0.0f));
			const glm::vec3 vb = glm::normalize(glm::vec3(0.0f, (s12 - s10) / (myTileSize * myTileResolution), 2 * delta));
			const auto normal = glm::cross(vb, va);

			temporaryElements[j + i * myTileSize].myNormal = normal;
		}
	}

	// should not take long as size was already reserved
	for (auto element : temporaryElements)
	{
		aTile->AddTerrainElement(element);
	}

	aTile->OnFinishBuild();
}

TerrainTileBuildingTask::~TerrainTileBuildingTask()
{
}

float TerrainTileBuildingTask::SamplePerlinNoise(float x, float y)
{
	auto tileNoise = 0.f;

	//auto lerpFactor = glm::smoothstep(0.3, 0.6, myPerlin.noise(x * scale / 1024.f, y * scale / 1024.f));
	auto lerpFactor = myWorldCell->IsFlag(TerrainGeneration::PointTypeFlags::Mountain) ? 0.f : 1.f;

	const auto mountainHeight = 2.f * (0.5f + 0.5f * myPerlin.noise(x * scale / 256.f, y * scale / 256.f));
	const auto plainHeight = 1.f;

	// warping the mountains to mask the 8 axis of the perlin noise
	const auto warpScale = 40.f / scale;
	auto warpX = warpScale * myPerlin.noise(x*scale / 40.f + 0.3f, y*scale / 40.f + 0.3f) - 0.5f;
	auto warpY = warpScale * myPerlin.noise(x*scale / 40.f + 0.3f, y*scale / 40.f + 0.3f, 1) - 0.5f;

	auto hardNoiseModifier = 1.f;
	float freq = 1.f;
	float amp = 1.f;

	// Noise computation
	for (int d = 1; d <= myNoiseDepth; d++)
	{
		freq *= lacunarity;
		amp *= gain;

		auto softNoise = 0.f;
		auto hardNoise = 0.f;

		if (lerpFactor > 0.f)
		{
			softNoise = myPerlin.noise(freq * x * scale / 384.f, freq * y * scale / 384.f);
		}

		if (lerpFactor < 1.f)
		{
			auto n = myPerlin.noise(freq * (x + warpX) * scale / 256.f, freq * (y + warpY) * scale / 256.f) * 2.f - 1.f;
			// C-infinity abs approximation
			hardNoise = 1.f - abs(60.f * n * n * n / (0.1f + 60.f * n * n)) + 0.5f;
		}

		tileNoise +=(lerpFactor * softNoise * amp * plainHeight + hardNoise * hardNoiseModifier * (1 - lerpFactor) * mountainHeight);
		hardNoiseModifier *= 0.85f * gain;

		warpX *= 0.25f;
		warpY *= 0.25f;
	}

	return locMultiplier * tileNoise;
}

TerrainTileBuilder::TerrainTileBuilder(const int aTileSize, const float aResolution, unsigned int aSeed):
	myTileSize(aTileSize),
	myTileResolution(aResolution),
	mySeed(aSeed)
{
}

void TerrainTileBuilder::BuildTileRequest(TerrainTile* aTile)
{
	if (myLoadingTasks.size() < myMaximumThreadLoad)
	{
		myLoadingTasks.push_back(new TerrainTileBuildingTask(mySeed, myTileSize, myTileResolution, aTile));
	}
	else
	{
		myLoadingQueue.push(aTile);
	}
}

void TerrainTileBuilder::Update()
{
	auto it = myLoadingTasks.begin();
	while (it < myLoadingTasks.end())
	{
		if ((*it)->myHandle.wait_for(std::chrono::seconds(0)) == std::future_status::ready)
		{
			delete *it;
			it = myLoadingTasks.erase(it);
		}
		else
		{
			++it;
		}
	}

	for (int i = 0; i < myMaximumThreadLoad - myLoadingTasks.size(); ++i)
	{
		if (!myLoadingQueue.empty())
		{
			myLoadingTasks.push_back(new TerrainTileBuildingTask(mySeed, myTileSize, myTileResolution, myLoadingQueue.front()));
			myLoadingQueue.pop();
		}
	}
}
