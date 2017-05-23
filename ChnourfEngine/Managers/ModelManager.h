#pragma once
#include <map>
#include "ShaderManager.h"
#include "../Rendering/Models/Model.h"

using namespace Rendering;

class TerrainCell;

class Camera;

namespace Manager
{
	class ModelManager
	{
	public:
		ModelManager();
		~ModelManager();

		void FillScene(const ShaderManager* aShaderManager);

		void Draw(const glm::mat4& aCameraTransform, const ShaderManager* aShaderManager);
		void DrawShadowMap(const GLuint aShadowMapProgram);
		void Update();
		void DeleteModel(const std::string& gameModelName);
		const IGameObject& GetModel(const std::string& gameModelName) const;

		inline void ResetCulling()
		{
			for (auto model : gameModelList)
			{
				model.second->isVisible = true;
			}
		}

		void CullScene(const Camera& aCamera);

		void AddTerrainCell(const TerrainCell* aCell, int aCellSize, float aResolution);

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