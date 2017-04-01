#pragma once
#include <map>
#include "ShaderManager.h"
#include "../Rendering/Models/Model.h"

using namespace Rendering;

namespace Manager
{
	class ModelManager
	{
	public:
		ModelManager(ShaderManager* aShaderManager);
		~ModelManager();

		void Draw(const glm::mat4& aCameraTransform, ShaderManager* aShaderManager);
		void Update();
		void DeleteModel(const std::string& gameModelName);
		const IGameObject& GetModel(const std::string& gameModelName) const;

	private:
		std::map<std::string, IGameObject*> gameModelList; //store all the objects, to be optimised
	};
}