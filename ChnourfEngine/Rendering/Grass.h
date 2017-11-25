#include <future>
#include <random>
#include <vector>
#include "Models/TerrainTileModel.h"
//#include "../WorldGenerator/TerrainTile.h"

struct TerrainVertex;
class TerrainTile;

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
	Grass(unsigned int aTileSize, float aResolution, int aSeed);
	~Grass();

	void Draw(const Manager::ShaderManager* aShaderManager, const vec2i& aTileIndex, GLuint aGrassTexture);
	void Update(bool aMustGenerate, const TerrainTile* aTile);
	void Reset();

	__forceinline bool IsGenerated() const{ return myIsGenerated; }

private:
	void OnGrassGenerationComplete();
	void GenerateGrass(const TerrainTile* aTile);

	std::vector<GrassInstance> myGrassData;
	unsigned int myDensityPerSqMeter;
	unsigned int myTileSize;
	int mySeed;
	std::default_random_engine myRandomEngine;
	float myResolution;

	GLuint myVAO;
	GLuint myVBO;
	GLuint myInstanceVBO;
	GLuint myProgram;

    std::future<void> myGeneratingTask;

	bool myIsGenerated;
	bool myIsGenerating;
};
