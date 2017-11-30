#include "InputManager.h" 
#include<iostream>
#include<fstream>
#include<vector>
#include "glm\glm.hpp"
#include "../Dependencies/GLFW/glfw3.h"
#include "../Core/Init/GLFWwrapper.h"
#include "../Dependencies/imgui/imgui_impl_glfw_glew.h"

using namespace Manager;

InputManager::InputManager()
{
	const auto window = Core::Init::GLFWwrapper::GetWindow();
	glfwSetKeyCallback(window, KeyboardCallback);
	glfwSetCursorPosCallback(window, CursorPosCallback);
	glfwSetMouseButtonCallback(window, MouseCallback);

	//glutMotionFunc(MotionCallback);
	//glfwSetCursorPosCallback(window, MotionCallback);
}

void InputManager::KeyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (action == GLFW_PRESS)
	{
		GetInstance()->myInputState.keyboardState[key] = true;
		GetInstance()->OnKeyPressedSlot(key);
	}
	if (action == GLFW_RELEASE)
	{
		GetInstance()->myInputState.keyboardState[key] = false;
		GetInstance()->OnKeyReleasedSlot(key);
	}

	ImGui_ImplGlfwGL3_KeyCallback(window, key, scancode, action, mods);
}

bool enableMotion = false;

void InputManager::MouseCallback(GLFWwindow* window, int button, int action, int mods)
{
	if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS)
	{
		enableMotion = true;
	}
	else
	{
		enableMotion = false;
	}

	double xpos, ypos;
	glfwGetCursorPos(window, &xpos, &ypos);

	GetInstance()->myInputState.cursorPosition.x = xpos;
	GetInstance()->myInputState.cursorPosition.y = ypos;

	ImGui_ImplGlfwGL3_MouseButtonCallback(window, button, action, mods);
}

void InputManager::CursorPosCallback(GLFWwindow* window, double xpos, double ypos)
{
	if (enableMotion)
	{
		float deltaX = ((float)(xpos - GetInstance()->myInputState.cursorPosition.x)) * 0.004f;
		GetInstance()->myInputState.cursorPosition.x = xpos;
		float deltaY = ((float)(ypos - GetInstance()->myInputState.cursorPosition.y)) * 0.004f;
		GetInstance()->myInputState.cursorPosition.y = ypos;
		GetInstance()->OnMouseMotionSlot(deltaX, deltaY);

		GetInstance()->myInputState.cursorPosition.x = xpos;
		GetInstance()->myInputState.cursorPosition.y = ypos;

	}
}