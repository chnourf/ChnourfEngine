#include "Camera.h"
#include "glm\gtc\matrix_transform.hpp"

Camera::Camera()
{
	myCameraPos = glm::vec3(0.0f, 2.0f, 5.0f);
	myCameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
	myCameraUp = glm::vec3(0.0f, 1.0f, 0.0f);


	//glm::vec3 cameraTarget = glm::vec3(0.0f, 0.0f, 0.0f);
	//glm::vec3 cameraDirection = glm::normalize(cameraPos - cameraTarget);

	//glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
	//glm::vec3 cameraRight = glm::normalize(glm::cross(up, cameraDirection));

	//glm::vec3 cameraUp = glm::cross(cameraDirection, cameraRight);
}

void Camera::Initialize(int aWindowWidth, int aWindowHeight)
{
	myProjectionMatrix = glm::perspective(45.0f, (float)aWindowWidth / (float)aWindowHeight, 0.1f, 3000.f);
}

void Camera::Update()
{
	auto viewMatrix = glm::lookAt(myCameraPos, myCameraPos + myCameraFront, myCameraUp);

	auto cameraMatrix = myProjectionMatrix * viewMatrix;

	auto nearPlane = Plane(vec3f(cameraMatrix[0][3] + cameraMatrix[0][2], cameraMatrix[1][3] + cameraMatrix[1][2], cameraMatrix[2][3] + cameraMatrix[2][2]), cameraMatrix[3][3] + cameraMatrix[3][2]);
	auto farPlane = Plane(vec3f(cameraMatrix[0][3] - cameraMatrix[0][2], cameraMatrix[1][3] - cameraMatrix[1][2], cameraMatrix[2][3] - cameraMatrix[2][2]), cameraMatrix[3][3] - cameraMatrix[3][2]);

	auto leftPlane = Plane(vec3f(cameraMatrix[0][3] + cameraMatrix[0][0], cameraMatrix[1][3] + cameraMatrix[1][0], cameraMatrix[2][3] + cameraMatrix[2][0]), cameraMatrix[3][3] + cameraMatrix[3][0]);
	auto rightPlane = Plane(vec3f(cameraMatrix[0][3] - cameraMatrix[0][0], cameraMatrix[1][3] - cameraMatrix[1][0], cameraMatrix[2][3] - cameraMatrix[2][0]), cameraMatrix[3][3] - cameraMatrix[3][0]);

	auto bottomPlane = Plane(vec3f(cameraMatrix[0][3] + cameraMatrix[0][1], cameraMatrix[1][3] + cameraMatrix[1][1], cameraMatrix[2][3] + cameraMatrix[2][1]), cameraMatrix[3][3] + cameraMatrix[3][1]);
	auto topPlane = Plane(vec3f(cameraMatrix[0][3] - cameraMatrix[0][1], cameraMatrix[1][3] - cameraMatrix[1][1], cameraMatrix[2][3] - cameraMatrix[2][1]), cameraMatrix[3][3] - cameraMatrix[3][1]);

	// not 100% sure
	myFrustum = Frustum(nearPlane, farPlane, leftPlane, rightPlane, bottomPlane, topPlane);

	//GLfloat radius = 7.0f;
	////GLfloat camX = sin(glutGet(GLUT_ELAPSED_TIME) / 5000.f) * radius;
	//GLfloat camX = 0;
	////GLfloat camZ = cos(glutGet(GLUT_ELAPSED_TIME) / 5000.f) * radius;
	//GLfloat camZ = radius;
	//myTransform = glm::lookAt(glm::vec3(camX, 0.0, camZ), glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.0, 1.0, 0.0));
	//myTransform = glm::lookAt(glm::vec3(0, 0.0, -10), glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.0, 1.0, 0.0));
}