//Init_GLEW.h
#pragma once
#include <iostream>
#include "../../Dependencies/glew/glew.h"
#include "../../Dependencies/freeglut/freeglut.h"

namespace Core 
{
	namespace Init 
	{
		inline void Init_GLEW()
		{
			glewExperimental = true;
			if (glewInit() == GLEW_OK)
			{
				std::cout << "GLEW: Initialize" << std::endl;
			}

			if (glewIsSupported("GL_VERSION_4_5"))
			{
				std::cout << "GLEW GL_VERSION_4_5 is 4.5\n " << std::endl;
			}
			else
			{
				std::cout << " GLEW GL_VERSION_4_5 not supported\n " << std::endl;
			}
		}
	}
}