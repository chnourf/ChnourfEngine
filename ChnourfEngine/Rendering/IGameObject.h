#pragma once
#include <vector>
#include <iostream>
#include "../Dependencies/glew/glew.h"
#include "../Dependencies/freeglut/freeglut.h"
#include "../Dependencies/SOIL/SOIL.h"
#include "VertexFormat.h"
#include "../Core/Intersection.h"
#include "../Core/UID.h"

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
		virtual void DrawForShadowMap(const Manager::ShaderManager* aShaderManager) = 0;
		virtual void Update() = 0;
		virtual void SetProgram(GLuint aShaderName) = 0;
		virtual void Destroy() = 0;

		const AABB& GetAABB() const { return myAABB; }
		const vec3f& GetPosition() const { return myPosition; }

		bool isVisible = true;

	protected:
		UID myUID;
		AABB myAABB;
		vec3f myPosition;
	};

	inline IGameObject::~IGameObject()
	{//blank
	}
}