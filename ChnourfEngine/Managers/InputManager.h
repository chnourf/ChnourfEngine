#pragma once

#include "../Dependencies/glew/glew.h"
#include "../Dependencies/freeglut/freeglut.h"
#include "../Core/Singleton.h"
#include "../Core/Slot.h"
#include "../Core/Vector.h"

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
		static void KeyboardDownCallback(unsigned char key, int x, int y);
		static void KeyboardUpCallback(unsigned char key, int x, int y);
		static void MouseCallback(int button, int state, int x, int y);
		static void MotionCallback(int x, int y);
	};
}