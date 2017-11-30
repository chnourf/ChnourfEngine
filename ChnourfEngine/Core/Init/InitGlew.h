//Init_GLEW.h
#pragma once
#include <iostream>
#include "../../Dependencies/glew/glew.h"
#include <cassert>

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
			else
			{
				assert(false);
			}

			if (glewIsSupported("GL_VERSION_3_3"))
			{
				std::cout << "GLEW GL_VERSION_3_3 is 3.3\n " << std::endl;
			}
			else
			{
				std::cout << " GLEW GL_VERSION_3_3 not supported\n " << std::endl;
			}
		}
	}
}