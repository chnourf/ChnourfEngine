#pragma once
#include "ShaderManager.h"
#include "../Rendering/Models/Model.h"

#include <future>

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
		void DrawShadowMap(const ShaderManager* aShaderManager);
		void Update();
		const IGameObject& GetModel(const UID& anUID) const;

		void ResetCulling();

		void CullScene(const Camera& aCamera);

		void AddTerrainCell(const TerrainCell* aCell, int aCellSize, float aResolution);
		void RemoveTerrainCell(const vec2i& anIndex);

		inline const std::vector<TextureFormat>& GetLoadedTextures() { return myLoadedTextures; }
		inline void AddLoadedTexture(const TextureFormat& aTexture) {
			//check if it is not already here
			myLoadedTextures.push_back(aTexture);
		}

	private:
		std::vector<IGameObject*> gameModelList; //store all the objects, to be optimised
		std::vector<TextureFormat> myLoadedTextures;

		std::vector<std::future<void>> myCullingTasks;
	};
}