#pragma once

#include <memory>

#include "ShaderManager.h"
#include "../Rendering/Camera.h"
#include "../Rendering/Light/DirectionalLight.h"
#include "../Core/Init/IListener.h"
#include "../Core/Singleton.h"

namespace Manager
{
	class ModelManager;
	class ShaderManager;

	class SceneManager : public Core::IListener, public Singleton<SceneManager>
	{
		//friend class Singleton<SceneManager>;

	public:
		~SceneManager();

		void NotifyBeginFrame() override;
		void NotifyDisplayFrame() override;
		void NotifyEndFrame() override;
		void NotifyReshape(int aWidth, int aHeight, int aPrevious_width, int aPrevious_height) override;

		SceneManager();

	private:
		static void KeyboardCallback(unsigned char key, int x, int y); // to be moved to input manager
		static void MouseCallback(int button, int state, int x, int y);
		static void MotionCallback(int x, int y);

		std::unique_ptr<ModelManager> myModelManager;
		std::unique_ptr<ShaderManager> myShaderManager;
		Camera myCurrentCamera;
		DirectionalLight myPointLight;
	};
}