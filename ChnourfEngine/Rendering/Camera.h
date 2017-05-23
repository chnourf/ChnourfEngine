#pragma once

#include "glm\glm.hpp"
#include "IGameObject.h"

#include "../Core/Vector.h"
#include "../Core/Intersection.h"

class Camera
{
public:
	Camera();

	void Initialize(int aWindowWidth, int aWindowHeight);

	void Update();

	//glm::mat4 myTransform;

	glm::vec3 myCameraPos;// = glm::vec3(0.0f, 0.0f, 15.0f);
	glm::vec3 myCameraFront;// = glm::vec3(0.0f, 0.0f, -1.0f);
	glm::vec3 myCameraUp;// = glm::vec3(0.0f, 1.0f, 0.0f);

	inline const glm::mat4& GetProjectionMatrix() const { return myProjectionMatrix; }
	inline const Frustum& GetFrustum() const { return myFrustum; }

private:
	glm::mat4 myProjectionMatrix;
	Frustum myFrustum;
};