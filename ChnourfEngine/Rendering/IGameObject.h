#pragma once
#include <vector>
#include <iostream>
#include "../Dependencies/glew/glew.h"
#include "../Dependencies/freeglut/freeglut.h"
#include "../Dependencies/SOIL/SOIL.h"
#include "VertexFormat.h"

namespace Manager
{
	class ShaderManager;
}

namespace Rendering
{
	class IGameObject
	{
	public:
		virtual ~IGameObject() = 0;

		virtual void Draw(const Manager::ShaderManager* aShaderManager) = 0;
		virtual void DrawForShadowMap(const GLuint aShadowMapProgram) = 0;
		virtual void Update() = 0;
		virtual void SetProgram(GLuint aShaderName) = 0;
		virtual void Destroy() = 0;
	};

	inline IGameObject::~IGameObject()
	{//blank
	}
}