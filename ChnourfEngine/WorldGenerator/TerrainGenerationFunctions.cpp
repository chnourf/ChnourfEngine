#include "TerrainGenerationFunctions.h"
#include "TerrainTile.h"
#include <algorithm>
#include <array>
#include <vector>
#include <random>

namespace TerrainGeneration
{
	Biome DeduceBiome(const float aTemperature, const float aRainfall)
	{
		if (aTemperature < 0.25f)
		{
			if (aRainfall > 0.5f) // snow
			{
				return Biome::Snow;
			}
			else if (aRainfall > 0.35f) // tundra
			{
				return Biome::Tundra;
			}
			else if (aRainfall > 0.15f) // bare
			{
				return Biome::Bare;
			}
			else // scorched
			{
				return Biome::Scorched;
			}
		}
		else if (aTemperature < 0.5f)
		{
			if (aRainfall > 0.66f) // taiga
			{
				return Biome::Taiga;
			}
			else if (aRainfall > 0.33f) // shrubland
			{
				return Biome::Shrubland;
			}
			else // temperate desert
			{
				return Biome::TemperateDesert;
			}
		}
		else if (aTemperature < 0.75)
		{
			if (aRainfall > 0.85f) // temperate rain forest
			{
				return Biome::TemperateRainForest;
			}
			else if (aRainfall > 0.5f) // temperate deciduous forest
			{
				return Biome::TemperateDeciduousForest;
			}
			else if (aRainfall > 0.15f)
			{
				return Biome::Grassland; // grassland
			}
			else
			{
				return Biome::TemperateDesert; // temperate desert
			}
		}
		else
		{
			if (aRainfall > 0.65f)
			{
				return Biome::TropicalRainForest; // tropical rain forest
			}
			else if (aRainfall > 0.35f)
			{
				return Biome::TemperateRainForest; // temperate rain forest
			}
			else if (aRainfall > 0.15f)
			{
				return Biome::Grassland; // grassland
			}
			else
			{
				return Biome::SubtropicalDesert; // subtropical desert
			}
		}
	}

	LandscapeType GetRelief(const float anAvgAltitude)
	{
		if (anAvgAltitude < locLandscapeRepartitionPercentages[0])
		{
			return LandscapeType::Sea;
		}

		if (anAvgAltitude < locLandscapeRepartitionPercentages[1])
		{
			return LandscapeType::Plain;
		}

		if (anAvgAltitude < locLandscapeRepartitionPercentages[2])
		{
			return LandscapeType::Hills;
		}

		if (anAvgAltitude < locLandscapeRepartitionPercentages[3])
		{
			return LandscapeType::Mountains;
		}

		return LandscapeType::HighMountains;
	}

	void ComputeErosion(std::vector<TerrainElement>& elevationMap, const unsigned int iterations, const TerrainGeneration::ErosionParams& params, const unsigned int& aTileSize, std::default_random_engine aRandomEngine)
{
	std::uniform_int_distribution<int> distribution(0, aTileSize - 2);

	float Kq = params.Kq, Kevap = params.Kevap, Kerosion = params.Kerosion, Kdepos = params.Kdepos, Kinertia = params.Ki,
		minSlope = params.minSlope, Kgravity = params.g;

	const unsigned int MAX_PATH_LEN = aTileSize * 4;

#define DEPOSIT_AT(X, Z, W) \
	{ \
	float delta=ds*(W); \
	elevationMap [X + aTileSize * Z].myElevation  += delta; \
	}

#define DEPOSIT(H) \
	for (int z = zi - 1; z <= zi + 2; ++z)\
	{\
		if (z < 0 || z > aTileSize - 1)\
			continue;\
		float zo = z - zp;\
		float zo2 = zo*zo;\
		for (int x = xi - 1; x <= xi + 2; ++x)\
		{\
			if (x < 0 || x > aTileSize - 1)\
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
		int xi = distribution(aRandomEngine);
		int zi = distribution(aRandomEngine);

		float xp = xi, zp = zi;
		float xf = 0, zf = 0;

		float h = elevationMap[xi + aTileSize * zi].myElevation;
		float carriedSoil = 0, speed = 1, water = 1;

		float h00 = h;
		float h10 = elevationMap[xi + 1 + aTileSize * zi].myElevation;
		float h01 = elevationMap[xi + aTileSize * (zi + 1)].myElevation;
		float h11 = elevationMap[(xi + 1) + aTileSize * (zi + 1)].myElevation;

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
			int nxi = glm::clamp((int)std::floor(nxp), 0, (int)aTileSize - 1);
			int nzi = glm::clamp((int)std::floor(nzp), 0, (int)aTileSize - 1);

			// the drop falls off the tile
			if (nxi == aTileSize - 1 || nzi == aTileSize - 1 || nxi == 0 || nzi == 0)
				break;

			float nxf = nxp - nxi;
			float nzf = nzp - nzi;

			float nh00 = elevationMap[nxi + aTileSize * nzi].myElevation;
			float nh10 = elevationMap[nxi + 1 + aTileSize * nzi].myElevation;
			float nh01 = elevationMap[nxi + aTileSize * (nzi + 1)].myElevation;
			float nh11 = elevationMap[nxi + 1 + aTileSize * (nzi + 1)].myElevation;

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
          elevationMap[X + aTileSize * Z].myElevation -=delta; \
        }

				for (int z = zi - 1; z <= zi + 2; ++z)
				{
					if (z < 0 || z > aTileSize - 1)
						continue;

					float zo = z - zp;
					float zo2 = zo*zo;

					for (int x = xi - 1; x <= xi + 2; ++x)
					{
						if (x < 0 || x > aTileSize - 1)
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
}