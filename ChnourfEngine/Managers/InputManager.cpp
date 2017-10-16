#include "InputManager.h" 
#include<iostream>
#include<fstream>
#include<vector>
#include "glm\glm.hpp"

using namespace Manager;

InputManager::InputManager()
{
	glutSetKeyRepeat(GLUT_KEY_REPEAT_ON);
	glutKeyboardFunc(KeyboardDownCallback);
	glutKeyboardUpFunc(KeyboardUpCallback);
	glutMouseFunc(MouseCallback);
	glutMotionFunc(MotionCallback);
}

void InputManager::KeyboardDownCallback(unsigned char key, int x, int y)
{
	GetInstance()->myInputState.keyboardState[key] = true;
	GetInstance()->OnKeyPressedSlot(key);
}

void InputManager::KeyboardUpCallback(unsigned char key, int x, int y)
{
	GetInstance()->myInputState.keyboardState[key] = false;
	GetInstance()->OnKeyReleasedSlot(key);
}

void InputManager::MouseCallback(int button, int state, int x, int y)
{
	if (button == GLUT_LEFT_BUTTON) {

		// when the button is released
		if (state == GLUT_UP)
		{
			GetInstance()->myInputState.cursorPosition.x = -1;
			GetInstance()->myInputState.cursorPosition.y = -1;
		}
		else
		{
			GetInstance()->myInputState.cursorPosition.x = x;
			GetInstance()->myInputState.cursorPosition.y = y;
		}
	}
}

void InputManager::MotionCallback(int x, int y)
{
	// this will only be true when the left button is down
	if (GetInstance()->myInputState.cursorPosition.x >= 0 && GetInstance()->myInputState.cursorPosition.y >= 0) {

		// could be interesting to store windowInfo and make a relative deltaX ------------------------------------------------------------------
		float deltaX = ((float)(x - GetInstance()->myInputState.cursorPosition.x)) * 0.004f;
		GetInstance()->myInputState.cursorPosition.x = x;
		float deltaY = ((float)(y - GetInstance()->myInputState.cursorPosition.y)) * 0.004f;
		GetInstance()->myInputState.cursorPosition.y = y;

		GetInstance()->OnMouseMotionSlot(deltaX, deltaY);
	}
}