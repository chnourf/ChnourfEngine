#include "TerrainCellBuilder.h"
#include"TerrainCell.h"
#include <time.h>
#include <iostream>

TerrainCellBuildingTask::TerrainCellBuildingTask(const int aSeed, const unsigned int aCellSize, TerrainCell* aCell):
	myCellSize(aCellSize),
	myNoiseDepth(6)
{
	myPerlin = PerlinNoise(aSeed);
	myHandle = std::async(std::launch::async, [this, aCell]() {BuildCell(aCell); });
}

void TerrainCellBuildingTask::BuildCell(TerrainCell* aCell)
{
	if (!aCell)
	{
		std::cout << "ERROR : cell given to build does not exist" << std::endl;
		return;
	}

	std::vector<TerrainElement> temporaryElements;
	temporaryElements.reserve(myCellSize * myCellSize);

	//computing elevation
	for (unsigned int i = 0; i < myCellSize; ++i) {     // y
		for (unsigned int j = 0; j < myCellSize; ++j) {  // x
			const float x = (float)j / ((float)myCellSize-1) + aCell->GetGridIndex().y;
			const float y = (float)i / ((float)myCellSize-1) + aCell->GetGridIndex().x;

			const auto cellNoise = SamplePerlinNoise(x, y);

			//ushort elevation = (ushort)(cellNoise* 0xffff);
			temporaryElements.push_back(TerrainElement((float)cellNoise, glm::vec3()));
		}
	}

	//computing normals based on elevation
	for (unsigned int i = 0; i < myCellSize; ++i) {     // y
		for (unsigned int j = 0; j < myCellSize; ++j) {  // x

			auto delta = 1.f / ((float)myCellSize - 1);
			const float x = (float)j * delta + aCell->GetGridIndex().y;
			const float y = (float)i * delta + aCell->GetGridIndex().x;

			const auto index = j + i * myCellSize;
			const auto elementElevation = temporaryElements[index].myElevation;

			const auto s01 = (j == 0) ? SamplePerlinNoise(x-delta, y) : temporaryElements[index - 1].myElevation;
			const auto s21 = (j == myCellSize - 1) ? SamplePerlinNoise(x + delta, y) : temporaryElements[index + 1].myElevation;
			const auto s10 = (i == 0) ? SamplePerlinNoise(x, y - delta) : temporaryElements[index - myCellSize].myElevation;
			const auto s12 = (i == myCellSize - 1) ? SamplePerlinNoise(x, y + delta) : temporaryElements[index + myCellSize].myElevation;
			const glm::vec3 va = glm::normalize(glm::vec3(2 * delta, s21 - s01, 0.0f));
			const glm::vec3 vb = glm::normalize(glm::vec3(0.0f, s12 - s10, 2 * delta));
			const auto normal = glm::cross(vb, va);

			temporaryElements[j + i * myCellSize].myNormal = normal;
		}
	}

	for (auto element : temporaryElements)
	{
		aCell->AddTerrainElement(element);
	}

	aCell->OnFinishBuild();
}

TerrainCellBuildingTask::~TerrainCellBuildingTask()
{
}

float TerrainCellBuildingTask::SamplePerlinNoise(const float x, const float y)
{
	auto cellNoise = 0.f;

	// Typical Perlin noise
	//for (int d = 1; d <= 1; d++)
	//{
	//	float factor = pow(2, d);
	//	cellNoise += 1 / factor * myPerlin.noise(factor*x, factor*y, 0);
	//}

	//sigmoid function, perhaps not the better choice
	auto lerpFactor = exp(-pow((myPerlin.noise(x / 16, y / 16, 0) - 0.6), 2) / 0.004);
	auto lerpNoise = 1 / (1 + exp(-15 * (myPerlin.noise( x / 8, y /8, -18) - 0.5)));
	lerpFactor *= lerpNoise;
	lerpFactor = 1 - lerpFactor;

	auto mountainHeight = 1;

	// Typical Perlin noise
	for (int d = 1; d <= myNoiseDepth; d++)
	{
		float factor = pow(2, d);
		float softNoise = myPerlin.noise(factor*x / 3, factor*y / 3, 0);
		auto hardNoise = (1 - abs(myPerlin.noise(factor*x/2, factor*y/2, 0) * 2 - 1)) / ((d + 2) / 3);
		cellNoise += 1 / factor*(lerpFactor*softNoise*0.5 + hardNoise*(1 - lerpFactor)*mountainHeight);
	}
	cellNoise *= (myPerlin.noise(x/4, y/4, 2) + 1) / 2;

	return 200*cellNoise;
}

TerrainCellBuilder::TerrainCellBuilder(int aCellSize):
	myCellSize(aCellSize)
{
	mySeed = time(NULL);
	srand(mySeed);
}

void TerrainCellBuilder::BuildCellRequest(TerrainCell* aCell)
{
	if (myLoadingTasks.size() < myMaximumThreadLoad)
	{
		myLoadingTasks.push_back(new TerrainCellBuildingTask(mySeed, myCellSize, aCell));
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
			myLoadingTasks.push_back(new TerrainCellBuildingTask(mySeed, myCellSize, myLoadingQueue.front()));
			myLoadingQueue.pop();
		}
	}
}
