#include "ModelManager.h"
#include <string>

using namespace Manager;
using namespace Rendering;

ModelManager::ModelManager()
{

}

void ModelManager::FillScene(const ShaderManager* aShaderManager)
{
	glm::mat4 modelTrans;
	Models::Model* batman = new Models::Model("Data/Nanosuit/nanosuit2.3ds", modelTrans);
	batman->SetProgram(aShaderManager->GetShader("colorShader"));
	auto key = "batman";
	gameModelList[key] = batman;
}

ModelManager::~ModelManager()
{
	for (auto model : gameModelList)
	{
		delete model.second;
	}
	gameModelList.clear();
}

void ModelManager::DeleteModel(const std::string& gameModelName)
{
	IGameObject* model = gameModelList[gameModelName];
	model->Destroy();
	gameModelList.erase(gameModelName);
}

const IGameObject& ModelManager::GetModel(const std::string& gameModelName) const
{
	return (*gameModelList.at(gameModelName));
}

void ModelManager::Update()
{
	for (auto model : gameModelList)
	{
		model.second->Update();
	}
}

//should be done in a renderer
void ModelManager::Draw(const glm::mat4& aCameraTransform,const ShaderManager* aShaderManager)
{
	for (auto model : gameModelList)
	{
		model.second->Draw(aShaderManager);
	}
}