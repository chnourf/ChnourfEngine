#include "ModelManager.h"
#include "../Core/Math.h"
#include <string>

#include "../Core/Intersection.h"

#include "SceneManager.h"
#include "../Rendering/Models/TerrainCellModel.h"
#include "../WorldGenerator/TerrainCell.h"

using namespace Manager;
using namespace Rendering;

const unsigned int locCullThreads = 4;

ModelManager::ModelManager()
{
}

void ModelManager::FillScene(const ShaderManager* aShaderManager)
{
	glm::vec3 grassPositions[] = {
		glm::vec3(-1.0f,  0.0f,  1.0f),
		glm::vec3(2.0f,  5.0f, -15.0f),
		glm::vec3(-1.5f, -2.2f, -2.5f),
		glm::vec3(-3.8f, -2.0f, -12.3f),
		glm::vec3(2.4f, -0.4f, -3.5f),
		glm::vec3(-1.7f,  3.0f, -7.5f),
		glm::vec3(1.3f, -2.0f, -2.5f),
		glm::vec3(1.5f,  2.0f, -2.5f),
		glm::vec3(1.5f,  0.2f, -1.5f),
		glm::vec3(-1.3f,  1.0f, -1.5f)
	};

	for (int i = 0; i < 10; ++i)
	{
		glm::mat4 grassModelTrans;
		grassModelTrans = glm::translate(grassModelTrans, grassPositions[i]);

		// instancing would be useful here
		Models::Model* grass = new Models::Model("Data/Grass/grassPlanes.obj", grassModelTrans);
		grass->SetProgram(aShaderManager->GetShader("transparentShader"));
		grass->Create();
		auto key = "grass" + std::to_string(i);
		gameModelList.push_back(grass);
	}

	glm::mat4 modelTrans;
	modelTrans = glm::scale(modelTrans, glm::vec3(0.3f));
	modelTrans = glm::rotate(modelTrans, (float)M_PI_2, glm::vec3(-1.f, 0.f, 0.f));
	Models::Model* batman = new Models::Model("Data/Nanosuit/nanosuit2.3ds", modelTrans);
	batman->SetProgram(aShaderManager->GetShader("colorShader"));
	batman->Create();
	auto Nanosuitkey = "batman";
	//gameModelList[Nanosuitkey] = batman;

	glm::mat4 terrainTrans;
	terrainTrans = glm::scale(terrainTrans, glm::vec3(10.f));
	Models::Model* terrainTest = new Models::Model("Data/TerrainTest/terrain.obj", terrainTrans);
	terrainTest->SetProgram(aShaderManager->GetShader("colorShader"));
	terrainTest->Create();
	auto terrainKey = "terrain";
	//gameModelList[terrainKey] = terrainTest;
}

ModelManager::~ModelManager()
{
	for (auto model : gameModelList)
	{
		delete model;
	}
	gameModelList.clear();
}

const IGameObject& ModelManager::GetModel(const UID& anUID) const
{
	// TO CHANGE
	return (*gameModelList.at(0));
}

void ModelManager::Update()
{
	for (auto model : gameModelList)
	{
		model->Update();
	}
}

void ModelManager::CullScene(const Camera& aCamera)
{
	// to optimize with quad tree
	
	myCullingTasks.clear();

	for (unsigned int i = 0; i < locCullThreads; ++i)
	{
		const auto frustum = aCamera.GetFrustum();

		myCullingTasks.push_back(std::async(std::launch::async, [this, frustum, i]() {
			const unsigned int stepSize = gameModelList.size() / locCullThreads;
			const auto lastIndex = (i == locCullThreads - 1) ? gameModelList.size() : (i + 1) * stepSize;
			for (auto it = gameModelList.begin() + i * stepSize; it < gameModelList.begin() + lastIndex; ++it)
			{
				if (!AABBvsFrustum((*it)->GetAABB(), frustum))
				{
					(*it)->isVisible = false;
				}
			} }));
	}
}

void ModelManager::AddTerrainCell(const TerrainCell* aCell, int aCellSize, float aResolution)
{
	Models::TerrainCellModel* terrain = new Models::TerrainCellModel(aCell, aCellSize, aResolution);
	gameModelList.push_back(terrain);	
}

void ModelManager::Draw(const glm::mat4& aCameraTransform,const ShaderManager* aShaderManager)
{
	for (auto& handle : myCullingTasks)
	{
		handle.wait();
	}

	for (auto model : gameModelList)
	{
		if (model->isVisible)
		{
			model->Draw(aShaderManager);
		}
	}
}

void ModelManager::DrawShadowMap(const GLuint aShadowMapProgram)
{
	for (auto model : gameModelList)
	{
		if (model->isVisible)
		{
			model->DrawForShadowMap(aShadowMapProgram);
		}
	}
}