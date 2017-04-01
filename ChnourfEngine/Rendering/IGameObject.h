#pragma once
#include <vector>
#include <iostream>
#include "../Dependencies/glew/glew.h"
#include "../Dependencies/freeglut/freeglut.h"
#include "../Dependencies/SOIL/SOIL.h"
#include "VertexFormat.h"

namespace Rendering
{
	class IGameObject
	{
	public:
		virtual ~IGameObject() = 0;

		virtual void Draw() = 0;
		virtual void Update() = 0;
		virtual void SetProgram(GLuint aShaderName) = 0;
		virtual void Destroy() = 0;

		virtual GLuint GetVao() const = 0;
		virtual const std::vector<GLuint>& GetVbos() const = 0;
		virtual const GLuint& GetProgram() = 0;
	};

	inline IGameObject::~IGameObject()
	{//blank
	}
}