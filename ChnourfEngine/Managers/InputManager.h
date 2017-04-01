#pragma once

#include "../Dependencies/glew/glew.h"
#include "../Dependencies/freeglut/freeglut.h"
#include "../Core/Singleton.h"

namespace Manager
{
	class InputManager : public Singleton<InputManager>
	{
	public:
		~InputManager() {};

	private:
		InputManager(){};
	};
}