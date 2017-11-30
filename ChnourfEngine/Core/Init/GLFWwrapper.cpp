#include "IListener.h"
#include "GLFWwrapper.h"
#include "../../Core/Init/InitGlew.h"
#include "../../Dependencies/GLFW/glfw3.h"
#include "../../Dependencies/imgui/imgui_impl_glfw_glew.h"
#include "../../Dependencies/imgui/imgui.h"
#include <cassert>

using namespace Core::Init;

Core::IListener* GLFWwrapper::listener = nullptr;
static GLFWwindow* mainWindow = nullptr;

GLFWwindow* GLFWwrapper::GetWindow()
{
	return mainWindow;
}

void GLFWwrapper::Init(int w, int h, const char* aWindowName)
{
	if (!glfwInit())
	{
		assert(false);
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
	mainWindow = glfwCreateWindow(w, h, aWindowName, NULL, NULL);
	if (!mainWindow)
	{
		assert(false);
	}

	glfwMakeContextCurrent(mainWindow);

	Init_GLEW();

	glfwSwapInterval(1);

	//glutSetOption(GLUT_MULTISAMPLE, 8);

	//glutInitDisplayMode(aFramebufferInfo.flags);
	//glutInitWindowPosition(aWindowInfo.positionX, aWindowInfo.positionY);
	//glutInitWindowSize(aWindowInfo.width, aWindowInfo.height);

	std::cout << "GLFW:initialized" << std::endl;

	//these callbacks are used for rendering
	//glutReshapeFunc(ReshapeCallback);

	//glEnable(GL_MULTISAMPLE); MSAA and HDR are incompatible

	glShadeModel(GL_SMOOTH);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glFrontFace(GL_CCW);

	//our method to display some info.
	PrintOpenGLInfo();

	ImGui_ImplGlfwGL3_Init(mainWindow, true);
}

//starts the rendering Loop
void GLFWwrapper::Run()
{
	std::cout << "GLFW:\t Start Running " << std::endl;
	while (!glfwWindowShouldClose(mainWindow))
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glClearColor(0.3f, 0.5f, 0.7f, 1.0f);

		glfwPollEvents();
		ImGui_ImplGlfwGL3_NewFrame();

		if (listener)
		{
			listener->NotifyBeginFrame();
			listener->NotifyDisplayFrame();
			ImGui::Render();
			glfwSwapBuffers(mainWindow);

			listener->NotifyEndFrame();
		}
	}

	Close();
}

void GLFWwrapper::Close()
{
	std::cout << "GLFW:\t Finished" << std::endl;
	glfwTerminate();

	glfwDestroyWindow(mainWindow);

	ImGui_ImplGlfwGL3_Shutdown();
}

void GLFWwrapper::GetWindowWidthAndHeight(int& w, int& h)
{
	glfwGetFramebufferSize(mainWindow, &w, &h);
}

//void GLFWwrapper::ReshapeCallback(int aWidth, int aHeight)
//{
//	if (listener)
//	{
//		listener->NotifyReshape(aWidth, aHeight, windowInformation.width,	windowInformation.height);
//	}
//
//	windowInformation.width = aWidth;
//	windowInformation.height = aHeight;
//
//	float ratio = 1.0f * aWidth / aHeight;
//
//	// Use the Projection Matrix
//	glMatrixMode(GL_PROJECTION);
//
//	// Reset Matrix
//	glLoadIdentity();
//
//	// Set the viewport to be the entire window
//	glViewport(0, 0, aWidth, aHeight);
//
//	// Set the correct perspective.
//	//gluPerspective(45, ratio, 1, 1000);
//
//	// Get Back to the Modelview
//	glMatrixMode(GL_MODELVIEW);
//}

void GLFWwrapper::PrintOpenGLInfo()
{
	const unsigned char* renderer = glGetString(GL_RENDERER);
	const unsigned char* vendor = glGetString(GL_VENDOR);
	const unsigned char* version = glGetString(GL_VERSION);

	std::cout << "******************************************************               ************************" << std::endl;
	std::cout << "GLFW:\tVendor : " << vendor << std::endl;
	std::cout << "GLFW:\tRenderer : " << renderer << std::endl;
	std::cout << "GLFW:\tOpenGl version: " << version << std::endl;
}

void GLFWwrapper::SetListener(Core::IListener*& iListener)
{
	listener = iListener;
}