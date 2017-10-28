#include "TerrainCellBuilder.h"
#include"TerrainCell.h"
#include"TerrainGenerationFunctions.h"
#include <time.h>
#include <iostream>
#include "../Core/Vector.h"

const float scale = .5f;
const float locMultiplier = 50.f/scale; // noise result between 0 and this value (in meters)
const float gain = 0.5f;
const float lacunarity = 1.90f;

TerrainCellBuildingTask::TerrainCellBuildingTask(const int aSeed, const unsigned int aCellSize, float aCellResolution, TerrainCell* anEmptyCell):
	myCellSize(aCellSize),
	myNoiseDepth(8),
	myCellResolution(aCellResolution)
{
	myPerlin = PerlinNoise(aSeed);
	myRandomEngine.seed(aSeed);
	myHandle = std::async(std::launch::async, [this, anEmptyCell]() {BuildCell(anEmptyCell);});
}

void TerrainCellBuildingTask::BuildCell(TerrainCell* aCell)
{
	if (!aCell)
	{
		std::cout << "ERROR : cell given to build does not exist" << std::endl;
		return;
	}

	float minHeight = std::numeric_limits<float>::max();
	float maxHeight = std::numeric_limits<float>::min();

	std::vector<TerrainElement> temporaryElements;
	temporaryElements.reserve(myCellSize * myCellSize);

	//computing elevation
	for (unsigned int i = 0; i < myCellSize; ++i) {     // y
		for (unsigned int j = 0; j < myCellSize; ++j) {  // x
			const float x = ((float)j / ((float)myCellSize-1) + aCell->GetGridIndex().y) * myCellSize * myCellResolution;
			const float y = ((float)i / ((float)myCellSize-1) + aCell->GetGridIndex().x) * myCellSize * myCellResolution;

			const auto cellNoise = SamplePerlinNoise(x, y);

			if (cellNoise < minHeight)
			{
				minHeight = cellNoise;
			}
			if (cellNoise > maxHeight)
			{
				maxHeight = cellNoise;
			}

			temporaryElements.push_back(TerrainElement(cellNoise, glm::vec3()));
		}
	}

	aCell->SetMinHeight(minHeight);
	aCell->SetMaxHeight(maxHeight);

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
	//TerrainGeneration::ComputeErosion(temporaryElements, 20000, params, myCellSize, myRandomEngine);

	//// lerping the edges of the tiles to ensure continuity
	//for (unsigned int i = 0; i < myCellSize; ++i) {
	//	for (unsigned int j = 0; j < myCellSize; ++j) {

	//		auto index = i + j * myCellSize;
	//		auto lerpFactor = glm::clamp(5.5f - abs(12.f * (float)i / (float)myCellSize - 6.f), 0.f, 1.f);
	//		lerpFactor *= glm::clamp(5.5f - abs(12.f * (float)j / (float)myCellSize - 6.f), 0.f, 1.f);
	//		auto& el = temporaryElements[index].myElevation;
	//		auto& bel = elementsBeforeErosion[index].myElevation;
	//		el = bel + lerpFactor * (el - bel);
	//	}
	//}

	//computing normals based on elevation
	for (unsigned int i = 0; i < myCellSize; ++i) {     // y
		for (unsigned int j = 0; j < myCellSize; ++j) {  // x

			auto delta = 1.f / ((float)myCellSize - 1);
			const float x = ((float)j * delta + aCell->GetGridIndex().y) * myCellSize * myCellResolution;
			const float y = ((float)i * delta + aCell->GetGridIndex().x) * myCellSize * myCellResolution;

			const auto index = j + i * myCellSize;

			// technically erosion should have changed the elevation of the edges but we suppose it's negligible
			const auto s01 = (j == 0) ? SamplePerlinNoise(x - delta * myCellSize * myCellResolution, y) : temporaryElements[index - 1].myElevation;
			const auto s21 = (j == myCellSize - 1) ? SamplePerlinNoise(x + delta * myCellSize * myCellResolution, y) : temporaryElements[index + 1].myElevation;
			const auto s10 = (i == 0) ? SamplePerlinNoise(x, y - delta * myCellSize * myCellResolution) : temporaryElements[index - myCellSize].myElevation;
			const auto s12 = (i == myCellSize - 1) ? SamplePerlinNoise(x, y + delta * myCellSize * myCellResolution) : temporaryElements[index + myCellSize].myElevation;
			const glm::vec3 va = glm::normalize(glm::vec3(2 * delta, (s21 - s01) / (myCellSize * myCellResolution), 0.0f));
			const glm::vec3 vb = glm::normalize(glm::vec3(0.0f, (s12 - s10) / (myCellSize * myCellResolution), 2 * delta));
			const auto normal = glm::cross(vb, va);

			temporaryElements[j + i * myCellSize].myNormal = normal;
		}
	}

	// should not take long as size was already reserved
	for (auto element : temporaryElements)
	{
		aCell->AddTerrainElement(element);
	}

	aCell->OnFinishBuild();
}

TerrainCellBuildingTask::~TerrainCellBuildingTask()
{
}

float TerrainCellBuildingTask::SamplePerlinNoise(float x, float y)
{
	auto cellNoise = 0.f;
	float freq = 1.f;
	float amp = 1.f;

	auto warpX = 300.f * myPerlin.noise(x*scale / 400.f + 0.3f, y*scale / 400.f + 0.3f) - 0.5f;
	auto warpY = 500.f * myPerlin.noise(x*scale / 400.f + 0.3f, y*scale / 400.f + 0.3f, 1) - 0.5f;

	for (int d = 1; d <= 8; d++)
	{
		freq *= lacunarity;
		amp *= gain;

		auto softNoise = 0.f;

		float dist = sqrt(x*x + y * y) * (0.65f + 0.35f * myPerlin.noise(x / 500.f, y / 500.f));

		softNoise = myPerlin.noise(freq * (x + warpX) * scale / 384.f, freq * (y + warpY) * scale / 384.f) * glm::clamp(0.001f * (1500.f - abs(dist)), 0.f, 1.0f);

		cellNoise += softNoise*amp;

	}

	if ((cellNoise > 0.45f && cellNoise < 0.52f) || cellNoise > 0.6f)
	{
		cellNoise += 1.5f * pow(myPerlin.noise((x + warpX) / 400.f, (y + warpY) / 400.f), 2);
	}

	return cellNoise;

	//auto cellNoise = 0.f;

	//auto lerpFactor = glm::smoothstep(0.3, 0.6, myPerlin.noise(x * scale / 1024.f, y * scale / 1024.f));

	//const auto mountainHeight = 2.f * (0.5f + 0.5f * myPerlin.noise(x * scale / 256.f, y * scale / 256.f));
	//const auto plainHeight = 1.f;

	//// warping the mountains to mask the 8 axis of the perlin noise
	//const auto warpScale = 40.f / scale;
	//auto warpX = warpScale * myPerlin.noise(x*scale / 40.f + 0.3f, y*scale / 40.f + 0.3f) - 0.5f;
	//auto warpY = warpScale * myPerlin.noise(x*scale / 40.f + 0.3f, y*scale / 40.f + 0.3f, 1) - 0.5f;

	//auto hardNoiseModifier = 1.f;
	//float freq = 1.f;
	//float amp = 1.f;

	//// Noise computation
	//for (int d = 1; d <= myNoiseDepth; d++)
	//{
	//	freq *= lacunarity;
	//	amp *= gain;

	//	auto softNoise = 0.f;
	//	auto hardNoise = 0.f;

	//	if (lerpFactor > 0.f)
	//	{
	//		softNoise = myPerlin.noise(freq * x * scale / 384.f, freq * y * scale / 384.f);
	//	}

	//	if (lerpFactor < 1.f)
	//	{
	//		auto n = myPerlin.noise(freq * (x + warpX) * scale / 256.f, freq * (y + warpY) * scale / 256.f) * 2.f - 1.f;
	//		// C-infinity abs approximation
	//		hardNoise = 1.f - abs(60.f * n * n * n / (0.1f + 60.f * n * n)) + 0.5f;
	//	}

	//	cellNoise +=(lerpFactor * softNoise * amp * plainHeight + hardNoise * hardNoiseModifier * (1 - lerpFactor) * mountainHeight);
	//	hardNoiseModifier *= 0.85f * gain;

	//	warpX *= 0.25f;
	//	warpY *= 0.25f;
	//}

	//return locMultiplier * cellNoise;
}

TerrainCellBuilder::TerrainCellBuilder(const int aCellSize, const float aResolution):
	myCellSize(aCellSize),
	myCellResolution(aResolution)
{
	mySeed = time(NULL);
	srand(mySeed);
}

void TerrainCellBuilder::BuildCellRequest(TerrainCell* aCell)
{
	if (myLoadingTasks.size() < myMaximumThreadLoad)
	{
		myLoadingTasks.push_back(new TerrainCellBuildingTask(mySeed, myCellSize, myCellResolution, aCell));
	}
	else
	{
		myLoadingQueue.push(aCell);
	}
}

void TerrainCellBuilder::Update()
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
			myLoadingTasks.push_back(new TerrainCellBuildingTask(mySeed, myCellSize, myCellResolution, myLoadingQueue.front()));
			myLoadingQueue.pop();
		}
	}
}
