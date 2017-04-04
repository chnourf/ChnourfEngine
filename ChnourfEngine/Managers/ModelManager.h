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
		ModelManager();
		~ModelManager();

		void FillScene(const ShaderManager* aShaderManager);

		void Draw(const glm::mat4& aCameraTransform, const ShaderManager* aShaderManager);
		void Update();
		void DeleteModel(const std::string& gameModelName);
		const IGameObject& GetModel(const std::string& gameModelName) const;

		inline const std::vector<TextureFormat>& GetLoadedTextures() { return myLoadedTextures; }
		inline void AddLoadedTexture(const TextureFormat& aTexture) {
			//check if it is not already here
			myLoadedTextures.push_back(aTexture);
		}

	private:
		std::map<std::string, IGameObject*> gameModelList; //store all the objects, to be optimised
		std::vector<TextureFormat> myLoadedTextures;
	};
}