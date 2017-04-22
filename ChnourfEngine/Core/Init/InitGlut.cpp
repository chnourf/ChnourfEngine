#include "InitGLUT.h"
#include "IListener.h"

using namespace Core::Init;

Core::IListener* Init_GLUT::listener = nullptr;
Core::WindowInfo Init_GLUT::windowInformation;

void Init_GLUT::Init(const Core::WindowInfo& aWindowInfo, const Core::ContextInfo& aContextInfo, const Core::FramebufferInfo& aFramebufferInfo)
{
	int fakeargc = 1;
	char *fakeargv[] = { "fake", NULL };
	glutInit(&fakeargc, fakeargv);

	if (aContextInfo.core)
	{
		glutInitContextVersion(aContextInfo.majorVersion, aContextInfo.minorVersion);
		glutInitContextProfile(GLUT_CORE_PROFILE);
	}
	else
	{
		glutInitContextProfile(GLUT_COMPATIBILITY_PROFILE);
	}

	glutSetOption(GLUT_MULTISAMPLE, 8);

	glutInitDisplayMode(aFramebufferInfo.flags);
	glutInitWindowPosition(aWindowInfo.positionX, aWindowInfo.positionY);
	glutInitWindowSize(aWindowInfo.width, aWindowInfo.height);

	glutCreateWindow(aWindowInfo.name.c_str());

	std::cout << "GLUT:initialized" << std::endl;

	//these callbacks are used for rendering
	glutIdleFunc(IdleCallback);
	glutCloseFunc(CloseCallback);
	glutDisplayFunc(DisplayCallback);
	glutReshapeFunc(ReshapeCallback);

	//init GLEW, this can be called in main.cpp
	Init::Init_GLEW();

	glEnable(GL_MULTISAMPLE);

	glEnable(GL_FRAMEBUFFER_SRGB);

	glShadeModel(GL_SMOOTH);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glFrontFace(GL_CCW);

	//cleanup
	glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_GLUTMAINLOOP_RETURNS);

	//our method to display some info. Needs contextInfo and windowinfo
	PrintOpenGLInfo(aWindowInfo, aContextInfo);

	windowInformation = aWindowInfo;
}

//starts the rendering Loop
void Init_GLUT::Run()
{
	std::cout << "GLUT:\t Start Running " << std::endl;
	glutMainLoop();
}

void Init_GLUT::Close()
{
	std::cout << "GLUT:\t Finished" << std::endl;
	glutLeaveMainLoop();
}

void Init_GLUT::IdleCallback(void)
{
	//do nothing, just redisplay
	glutPostRedisplay();
}

void Init_GLUT::DisplayCallback()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearColor(0.3f, 0.5f, 0.7f, 1.0f);

	if (listener)
	{
		listener->NotifyBeginFrame();
		listener->NotifyDisplayFrame();

		glutSwapBuffers();

		listener->NotifyEndFrame();
	}
}

void Init_GLUT::ReshapeCallback(int aWidth, int aHeight)
{
	if (windowInformation.isReshapable == true)
	{
		if (listener)
		{
			listener->NotifyReshape(aWidth, aHeight, windowInformation.width,	windowInformation.height);
		}

		windowInformation.width = aWidth;
		windowInformation.height = aHeight;

		float ratio = 1.0f * aWidth / aHeight;

		// Use the Projection Matrix
		glMatrixMode(GL_PROJECTION);

		// Reset Matrix
		glLoadIdentity();

		// Set the viewport to be the entire window
		glViewport(0, 0, aWidth, aHeight);

		// Set the correct perspective.
		gluPerspective(45, ratio, 1, 1000);

		// Get Back to the Modelview
		glMatrixMode(GL_MODELVIEW);
	}
}

void Init_GLUT::CloseCallback()
{
	Close();
}

void Init_GLUT::EnterFullscreen()
{
	glutFullScreen();
}

void Init_GLUT::ExitFullscreen()
{
	glutLeaveFullScreen();
}

void Init_GLUT::PrintOpenGLInfo(const Core::WindowInfo& aWindowInfo,	const Core::ContextInfo& aContextInfo)
{
	const unsigned char* renderer = glGetString(GL_RENDERER);
	const unsigned char* vendor = glGetString(GL_VENDOR);
	const unsigned char* version = glGetString(GL_VERSION);

	std::cout << "******************************************************               ************************" << std::endl;
	std::cout << "GLUT:Initialise" << std::endl;
	std::cout << "GLUT:\tVendor : " << vendor << std::endl;
	std::cout << "GLUT:\tRenderer : " << renderer << std::endl;
	std::cout << "GLUT:\tOpenGl version: " << version << std::endl;
}

void Init_GLUT::SetListener(Core::IListener*& iListener)
{
	listener = iListener;
}