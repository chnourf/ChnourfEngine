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

	void OnMouseMotion(const float deltaX, const float deltaY);

	void Update();

	glm::vec3 myCameraPos;
	glm::vec3 myNextCameraPos;
	glm::vec3 myCameraFront;
	glm::vec3 myCameraUp;

	inline const glm::mat4& GetProjectionMatrix() const { return myProjectionMatrix; }
	inline const Frustum& GetFrustum() const { return myFrustum; }

private:
	glm::mat4 myProjectionMatrix;
	Frustum myFrustum;

	void UpdateFromKeyboard();
};