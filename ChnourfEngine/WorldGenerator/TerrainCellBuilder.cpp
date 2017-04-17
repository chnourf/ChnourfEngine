#include "TerrainCellBuilder.h"
#include"TerrainCell.h"
#include <time.h>
#include <iostream>

TerrainCellBuildingTask::TerrainCellBuildingTask(const int aSeed, const unsigned int aCellSize, TerrainCell* aCell):
	myCellSize(aCellSize)
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

	for (unsigned int i = 0; i < myCellSize; ++i) {     // y
		for (unsigned int j = 0; j < myCellSize; ++j) {  // x
			const float x = (float)j / ((float)myCellSize) + aCell->GetGridIndex().x;
			const float y = (float)i / ((float)myCellSize) + aCell->GetGridIndex().y;

			auto cellNoise = 0.f;

			// Typical Perlin noise
			for (int d = 1; d <= myNoiseDepth; d++)
			{
				float factor = pow(2, d);
				cellNoise += 1 / factor * myPerlin.noise(factor*x, factor*y, 0);
			}

			ushort elevation = (ushort)(cellNoise * 0xffff);
			aCell->AddTerrainElement(TerrainElement(elevation));
		}
	}

	aCell->OnFinishBuild();
}

TerrainCellBuildingTask::~TerrainCellBuildingTask()
{
}

TerrainCellBuilder::TerrainCellBuilder(int aCellSize):
	myCellSize(aCellSize)
{
	unsigned int seed = time(NULL);
	srand(seed);
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
