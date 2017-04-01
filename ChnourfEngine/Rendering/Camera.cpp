#include "Camera.h"
#include "glm\gtc\matrix_transform.hpp"

Camera::Camera()
{
	glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);
	glm::vec3 cameraTarget = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec3 cameraDirection = glm::normalize(cameraPos - cameraTarget);

	glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
	glm::vec3 cameraRight = glm::normalize(glm::cross(up, cameraDirection));

	glm::vec3 cameraUp = glm::cross(cameraDirection, cameraRight);
}

void Camera::Update()
{
	//GLfloat radius = 7.0f;
	////GLfloat camX = sin(glutGet(GLUT_ELAPSED_TIME) / 5000.f) * radius;
	//GLfloat camX = 0;
	////GLfloat camZ = cos(glutGet(GLUT_ELAPSED_TIME) / 5000.f) * radius;
	//GLfloat camZ = radius;
	//myTransform = glm::lookAt(glm::vec3(camX, 0.0, camZ), glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.0, 1.0, 0.0));
	//myTransform = glm::lookAt(glm::vec3(0, 0.0, -10), glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.0, 1.0, 0.0));
}