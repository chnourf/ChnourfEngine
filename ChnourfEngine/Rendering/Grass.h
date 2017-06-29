#include <vector>
#include <random>
#include "Models/TerrainCellModel.h"
//#include "../WorldGenerator/TerrainCell.h"

struct TerrainVertex;
class TerrainCell;

struct GrassInstance
{
	float x, y, z;
	char nx8, ny8, nz8;
	char unused;
	char atlasIndex8;
	char direction8;
	char scale8;
    char colorLerp8;
};

class Grass
{
public:
	Grass(unsigned int aCellSize, float aResolution, int aSeed);

	void Draw(const Manager::ShaderManager* aShaderManager, const vec2i& aTileIndex, GLuint aGrassTexture);
	void GenerateGrass(const TerrainCell* aCell);
	void Reset();

private:
	void OnGrassGenerationComplete();

	std::vector<GrassInstance> myGrassData;
	unsigned int myDensityPerSqMeter;
	unsigned int myCellSize;
	int mySeed;
	std::default_random_engine myRandomEngine;
	float myResolution;

	GLuint myVAO;
	GLuint myVBO;
	GLuint myInstanceVBO;
	GLuint myProgram;

	bool myIsGenerated;
};
