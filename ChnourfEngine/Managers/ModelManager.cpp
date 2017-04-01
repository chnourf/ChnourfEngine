#include "ModelManager.h"
#include <string>

using namespace Manager;
using namespace Rendering;

ModelManager::ModelManager(ShaderManager* aShaderManager)
{
	glm::vec3 cubePositions[] = {
		glm::vec3(0.0f,  0.0f,  0.0f),
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

	for (GLuint i = 0; i < 10; i++)
	{
		glm::mat4 modelTrans;
		modelTrans = glm::translate(modelTrans, cubePositions[i]);
		GLfloat angle = 20.0f * i;
		modelTrans = glm::rotate(modelTrans, angle, glm::vec3(1.0f, 0.3f, 0.5f));

		auto material = Material(glm::vec3(.1f, .1f, .1f), glm::vec3(1.f, .5f, .31f), glm::vec3(.5f, .5f, .5f), 32.f);

		//crate game object
		Models::Model* crate = new Models::Model(modelTrans, material);
		crate->SetProgram(aShaderManager->GetShader("colorShader"));
		crate->Create();
		auto key = "crate" + std::to_string(i);
		gameModelList[key] = crate;
	}
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
void ModelManager::Draw(const glm::mat4& aCameraTransform, ShaderManager* aShaderManager)
{
	for (auto model : gameModelList)
	{
		model.second->Draw();
	}
}