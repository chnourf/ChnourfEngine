#pragma once

#include "../Dependencies/glew/glew.h"
#include "../Dependencies/glew/glew.h"
#include "../Core/Singleton.h"
#include "../Core/Slot.h"
#include "../Core/Vector.h"

struct GLFWwindow;

namespace Manager
{
	class InputManager : public Singleton<InputManager>
	{
	public:
		InputManager();
		const bool* GetKeyBoardState() const { return myInputState.keyboardState; }

		Slot<void(unsigned char)> OnKeyPressedSlot;
		Slot<void(unsigned char)> OnKeyReleasedSlot;
		Slot<void(int, int, int, int)> MouseSlot;
		Slot<void(float, float)> OnMouseMotionSlot;

	private:
		struct InputState {
			Vector2<int> cursorPosition;
			bool keyboardState[256];
		};
		InputState myInputState;

		// must be static for glut
		static void KeyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
		static void CursorPosCallback(GLFWwindow* window, double xpos, double ypos);
		static void MouseCallback(GLFWwindow* window, int button, int action, int mods);
	};
}