//#include "InitGLEW.h"
//
//using namespace Core::Init;
//
//void Init_GLEW::Init() {
//
//	glewExperimental = true;
//	if (glewInit() == GLEW_OK)
//	{
//		std::cout << "GLEW: Initialize" << std::endl;
//	}
//
//	if (glewIsSupported("GL_VERSION_4_5"))
//	{
//		std::cout << "GLEW GL_VERSION_4_5 is 4.5\n " << std::endl;
//	}
//	else
//	{
//		std::cout << " GLEW GL_VERSION_4_5 not supported\n " << std::endl;
//	}
//}