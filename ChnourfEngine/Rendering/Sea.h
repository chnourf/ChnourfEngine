#pragma once

#include "glm\glm.hpp"
#include "../Dependencies/glew/glew.h"

namespace Manager
{
	class SceneManager;
}

class Sea
{
	friend class Manager::SceneManager;
	Sea();

	void Render(GLuint aProgram);

	GLuint myVao;
	GLuint myVbo;
};