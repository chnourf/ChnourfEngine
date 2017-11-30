#pragma once

#include <memory>

#include "ShaderManager.h"
#include "../Rendering/Camera.h"
#include "../Rendering/Cubemap.h"
#include "../Rendering/Light/DirectionalLight.h"
#include "../Core/Init/IListener.h"
#include "../Core/Singleton.h"

namespace Manager
{
	class ModelManager;
	class ShaderManager;
	class TerrainManager;

	class SceneManager : public Core::IListener, public Singleton<SceneManager>
	{
		friend class Singleton<SceneManager>;

	public:
		~SceneManager();

		void Initialize();

		void NotifyBeginFrame() override;
		void NotifyDisplayFrame() override;
		void NotifyEndFrame() override;
		void NotifyReshape(int aWidth, int aHeight, int aPrevious_width, int aPrevious_height) override;
		glm::vec3 GetCamPos() const { return myCurrentCamera.myCameraPos; }

		inline ModelManager* GetModelManager() { return myModelManager.get(); }

	private:
		SceneManager();

		GLuint myViewConstantUbo;

		std::unique_ptr<ModelManager> myModelManager;
		std::unique_ptr<ShaderManager> myShaderManager;
		Camera myCurrentCamera;
		DirectionalLight myDirectionalLight;
		Cubemap mySkybox;

		GLuint fbo; //to be moved, used for framebuffer render
		GLuint shadowMapFBO;
		GLuint shadowMapTexture;
		unsigned int shadowMapResolution = 2048;
		GLuint textureColorBuffer;
		GLuint framebufferQuadVao;
		GLuint framebufferQuadVbo;

		GLuint myNoiseTexture;

		glm::mat4 myLightSpaceMatrix;
	};
}