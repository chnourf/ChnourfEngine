#include "Camera.h"
#include "glm\gtc\matrix_transform.hpp"

Camera::Camera()
{
	myCameraPos = glm::vec3(0.0f, 200.0f, 5.0f);
	myCameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
	myCameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
}

void Camera::Initialize(int aWindowWidth, int aWindowHeight)
{
	myProjectionMatrix = glm::perspective(45.0f, (float)aWindowWidth / (float)aWindowHeight, 0.1f, 3000.f);
}

void Camera::Update()
{
	// UGLY UGLY UGLY UGLY UGLY UGLY UGLY UGLY UGLY UGLY
	myCameraPos -= 100.f * myCameraFront;

	auto viewMatrix = glm::lookAt(myCameraPos, myCameraPos + myCameraFront, myCameraUp);

	auto cameraMatrix = myProjectionMatrix * viewMatrix;

	auto nearPlane = Plane(vec3f(cameraMatrix[0][3] + cameraMatrix[0][2], cameraMatrix[1][3] + cameraMatrix[1][2], cameraMatrix[2][3] + cameraMatrix[2][2]), cameraMatrix[3][3] + cameraMatrix[3][2]);
	auto farPlane = Plane(vec3f(cameraMatrix[0][3] - cameraMatrix[0][2], cameraMatrix[1][3] - cameraMatrix[1][2], cameraMatrix[2][3] - cameraMatrix[2][2]), cameraMatrix[3][3] - cameraMatrix[3][2]);

	auto leftPlane = Plane(vec3f(cameraMatrix[0][3] + cameraMatrix[0][0], cameraMatrix[1][3] + cameraMatrix[1][0], cameraMatrix[2][3] + cameraMatrix[2][0]), cameraMatrix[3][3] + cameraMatrix[3][0]);
	auto rightPlane = Plane(vec3f(cameraMatrix[0][3] - cameraMatrix[0][0], cameraMatrix[1][3] - cameraMatrix[1][0], cameraMatrix[2][3] - cameraMatrix[2][0]), cameraMatrix[3][3] - cameraMatrix[3][0]);

	auto bottomPlane = Plane(vec3f(cameraMatrix[0][3] + cameraMatrix[0][1], cameraMatrix[1][3] + cameraMatrix[1][1], cameraMatrix[2][3] + cameraMatrix[2][1]), cameraMatrix[3][3] + cameraMatrix[3][1]);
	auto topPlane = Plane(vec3f(cameraMatrix[0][3] - cameraMatrix[0][1], cameraMatrix[1][3] - cameraMatrix[1][1], cameraMatrix[2][3] - cameraMatrix[2][1]), cameraMatrix[3][3] - cameraMatrix[3][1]);

	myFrustum = Frustum(nearPlane, farPlane, leftPlane, rightPlane, bottomPlane, topPlane);

	// UGLY UGLY UGLY UGLY UGLY UGLY UGLY UGLY UGLY UGLY
	myCameraPos += 100.f * myCameraFront;
}