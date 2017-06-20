#include "TerrainCellBuilder.h"
#include"TerrainCell.h"
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
	myHandle = std::async(std::launch::async, [this, anEmptyCell]() {BuildCell(anEmptyCell); });
}

void TerrainCellBuildingTask::ComputeErosion(std::vector<TerrainElement>& elevationMap, const unsigned int iterations, const ErosionParams& params)
{
	std::uniform_int_distribution<int> distribution(0, myCellSize - 2);

	float Kq = params.Kq, Kevap = params.Kevap, Kerosion = params.Kerosion, Kdepos = params.Kdepos, Kinertia = params.Ki,
		minSlope = params.minSlope, Kgravity = params.g;

	const unsigned int MAX_PATH_LEN = myCellSize * 4;

#define DEPOSIT_AT(X, Z, W) \
	{ \
	float delta=ds*(W); \
	elevationMap [X + myCellSize * Z].myElevation  += delta; \
	}

#define DEPOSIT(H) \
	for (int z = zi - 1; z <= zi + 2; ++z)\
	{\
		if (z < 0 || z > myCellSize - 1)\
			continue;\
		float zo = z - zp;\
		float zo2 = zo*zo;\
		for (int x = xi - 1; x <= xi + 2; ++x)\
		{\
			if (x < 0 || x > myCellSize - 1)\
				continue;\
			float xo = x - xp;\
			float w = 1 - (xo*xo + zo2)*0.25f;\
			if (w <= 0) continue;\
			w *= 0.03978873577f;\
			DEPOSIT_AT(x, z, w)\
		}\
	}\
	(H)+=ds;

	for (unsigned int iter = 0; iter < iterations; ++iter)
	{
		int xi = distribution(myRandomEngine);
		int zi = distribution(myRandomEngine);

		float xp = xi, zp = zi;
		float xf = 0, zf = 0;

		float h = elevationMap[xi + myCellSize * zi].myElevation;
		float carriedSoil = 0, speed = 1, water = 1;

		float h00 = h;
		float h10 = elevationMap[xi + 1 + myCellSize * zi].myElevation;
		float h01 = elevationMap[xi + myCellSize * (zi + 1)].myElevation;
		float h11 = elevationMap[(xi + 1) + myCellSize * (zi + 1)].myElevation;

		float dx = 0, dz = 0;

		unsigned numMoves = 0;
		for (; numMoves < MAX_PATH_LEN; ++numMoves)
		{
			// calc gradient
			float gx = h00 + h01 - h10 - h11;
			float gz = h00 + h10 - h01 - h11;

			// calc next pos
			dx = (dx - gx)*Kinertia + gx;
			dz = (dz - gz)*Kinertia + gz;

			float dl = sqrtf(dx*dx + dz*dz);
			if (dl <= FLT_EPSILON)
			{
				// pick random dir
				float a = std::rand();
				dx = cosf(a);
				dz = sinf(a);
			}
			else
			{
				dx /= dl;
				dz /= dl;
			}

			float nxp = xp + dx;
			float nzp = zp + dz;

			// sample next height
			int nxi = glm::clamp((int)std::floor(nxp), 0, (int)myCellSize - 1);
			int nzi = glm::clamp((int)std::floor(nzp), 0, (int)myCellSize - 1);

			// the drop falls of the cell
			if (nxi == myCellSize - 1 || nzi == myCellSize - 1 || nxi == 0 || nzi == 0)
				break;

			float nxf = nxp - nxi;
			float nzf = nzp - nzi;

			float nh00 = elevationMap[nxi + myCellSize * nzi].myElevation;
			float nh10 = elevationMap[nxi + 1 + myCellSize * nzi].myElevation;
			float nh01 = elevationMap[nxi + myCellSize * (nzi + 1)].myElevation;
			float nh11 = elevationMap[nxi + 1 + myCellSize * (nzi + 1)].myElevation;

			float nh = (nh00*(1 - nxf) + nh10*nxf)*(1 - nzf) + (nh01*(1 - nxf) + nh11*nxf)*nzf;

			// if higher than current, try to deposit sediment up to neighbour height
			if (nh >= h)
			{
				float ds = (nh - h) + 0.001f;

				if (ds >= carriedSoil)
				{
					// deposit all sediment and stop
					ds = carriedSoil;
					DEPOSIT(h)
						carriedSoil = 0;
					break;
				}
				DEPOSIT(h)
					carriedSoil -= ds;
					speed = 0;
			}

			// compute transport capacity
			float dh = h - nh;

			speed += Kgravity * std::min(dh, 1.f);

			float q = std::max(dh, minSlope)*speed*water*Kq;

			// deposit/erode (don't erode more than dh)
			float ds = carriedSoil - q;
			if (ds >= 0)
			{
				// deposit
				ds *= Kdepos;
				DEPOSIT(dh)
					carriedSoil -= ds;
			}
			else
			{
				// erode
				ds *= -Kerosion;
				ds = std::min(ds, dh*0.99f);

#define ERODE(X, Z, W) \
        { \
          float delta=ds*(W); \
          elevationMap[X + myCellSize * Z].myElevation -=delta; \
        }

				for (int z = zi - 1; z <= zi + 2; ++z)
				{
					if (z < 0 || z > myCellSize - 1)
						continue;

					float zo = z - zp;
					float zo2 = zo*zo;

					for (int x = xi - 1; x <= xi + 2; ++x)
					{
						if (x < 0 || x > myCellSize - 1)
							continue;

						float xo = x - xp;

						float w = 1 - (xo*xo + zo2)*0.25f;
						if (w <= 0) continue;
						w *= 0.1591549430918953f;

						ERODE(x, z, w)
					}
				}

				dh -= ds;
#undef ERODE

				carriedSoil += ds;
			}

			// move to the neighbour
			water *= 1.f - Kevap;

			xp = nxp; zp = nzp;
			xi = nxi; zi = nzi;
			xf = nxf; zf = nzf;

			h = nh;
			h00 = nh00;
			h10 = nh10;
			h01 = nh01;
			h11 = nh11;
		}
	}

#undef DEPOSIT
#undef DEPOSIT_AT
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

	std::vector<TerrainElement> elementsBeforeErosion = temporaryElements;

	//computing erosion, could be moved to presets.txt
	ErosionParams params;
	params.Kq = 1.5f;
	params.Kevap = 0.1f;
	params.Kerosion = .9f;
	params.Kdepos = .02f;
	params.Ki = .01f;
	params.minSlope = 0.05f;
	params.g = 1.f;
	ComputeErosion(temporaryElements, 20000, params);

	// lerping the edges of the tiles to assure continuity
	for (unsigned int i = 0; i < myCellSize; ++i) {
		for (unsigned int j = 0; j < myCellSize; ++j) {

			auto index = i + j * myCellSize;
			auto lerpFactor = glm::clamp(5.5f - abs(12.f * (float)i / (float)myCellSize - 6.f), 0.f, 1.f);
			lerpFactor *= glm::clamp(5.5f - abs(12.f * (float)j / (float)myCellSize - 6.f), 0.f, 1.f);
			auto& el = temporaryElements[index].myElevation;
			auto& bel = elementsBeforeErosion[index].myElevation;
			el = bel + lerpFactor * (el - bel);
		}
	}

	//computing normals based on elevation
	for (unsigned int i = 0; i < myCellSize; ++i) {     // y
		for (unsigned int j = 0; j < myCellSize; ++j) {  // x

			auto delta = 1.f / ((float)myCellSize - 1);
			const float x = ((float)j * delta + aCell->GetGridIndex().y) * myCellSize * myCellResolution;
			const float y = ((float)i * delta + aCell->GetGridIndex().x) * myCellSize * myCellResolution;

			const auto index = j + i * myCellSize;

			// technically erosion should have changed the elevation of the edges but we supposed it's negligeable
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

	// adding grass RESERVE MEMORY FIRST
	for (unsigned int i = 0; i < 1000; ++i)
	{
		std::uniform_real_distribution<float> distribution(0.f, (float) (myCellSize - 1));
		float xf = distribution(myRandomEngine);
		float zf = distribution(myRandomEngine);
		auto grassPos = aCell->VerticalRaycast(vec2f(xf, zf));
		aCell->AddGrassPosition(grassPos);
	}

	aCell->OnFinishBuild();
}

TerrainCellBuildingTask::~TerrainCellBuildingTask()
{
}

float TerrainCellBuildingTask::SamplePerlinNoise(float x, float y)
{
	auto cellNoise = 0.f;

	auto lerpFactor = glm::smoothstep(0.3, 0.6, myPerlin.noise(x * scale / 1024.f, y * scale / 1024.f));

	const auto mountainHeight = 2.f * (0.5f + 0.5f * myPerlin.noise(x * scale / 256.f, y * scale / 256.f));
	const auto plainHeight = 1.f;

	// warping the mountains to mask the 8 axis of the perlin noise
	const auto warpScale = 40.f / scale;
	auto warpX = warpScale * myPerlin.noise(x*scale / 40.f, y*scale / 40.f) - 0.5f;
	auto warpY = warpScale * myPerlin.noise(x*scale / 40.f, y*scale / 40.f, 1) - 0.5f;

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

		cellNoise +=(lerpFactor * softNoise * amp * plainHeight + hardNoise * hardNoiseModifier * (1 - lerpFactor) * mountainHeight);
		hardNoiseModifier *= 0.85f * gain;

		warpX *= 0.25f;
		warpY *= 0.25f;
	}

	return locMultiplier * cellNoise;
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
