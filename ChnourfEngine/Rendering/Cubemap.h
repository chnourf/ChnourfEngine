#pragma once

#include "glm\glm.hpp"
#include "../Dependencies/glew/glew.h"

#include <string>

class Cubemap
{
public:
	Cubemap();

	void Render(GLuint aProgram, const glm::mat4& aViewTransform);

	void LoadTextures(const std::string& aPath);

	inline const GLuint GetVao() const { return myVao; }
	inline const GLuint GetVbo() const { return myVbo; }
	inline const GLuint GetTexture() const { return myTextureId; }

protected:
	GLuint myVao;
	GLuint myVbo;
	GLuint myTextureId;
};