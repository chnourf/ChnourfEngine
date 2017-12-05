#include "Camera.h"
#include "glm\gtc\matrix_transform.hpp"
#include "../Core/Time.h"
#include "../Managers/InputManager.h"
#include "glm\gtx\rotate_vector.hpp"
#include "../Dependencies/imgui/imgui.h"


Camera::Camera()
{
	//myCameraPos = glm::vec3(0.0f, 200.0f, 5.0f);
	myCameraPos = glm::vec3(1740.f, 400.0f, 140.0f);
	myNextCameraPos = myCameraPos;
	myCameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
	myCameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
}

void Camera::Initialize(int aWindowWidth, int aWindowHeight)
{
	myProjectionMatrix = glm::perspective(45.0f, (float)aWindowWidth / (float)aWindowHeight, 0.1f, 3000.f);
	Manager::InputManager::GetInstance()->OnMouseMotionSlot.Connect(std::bind(&Camera::OnMouseMotion, this, std::placeholders::_1, std::placeholders::_2));
}

auto speedMultiplier = 10.f;
void Camera::UpdateFromKeyboard()
{
	ImGui::Text("Camera Position : x %f, y %f, z %f", float(myCameraPos.x), float(myCameraPos.y), float(myCameraPos.z));
	ImGui::Text("Camera Facing : x %f, y %f, z %f",  float(myCameraFront.x), float(myCameraFront.y), float(myCameraFront.z));

	ImGui::SliderFloat("Camera Speed", &speedMultiplier, 0.01f, 500.f, "%.3f", 2.f);
	
	auto cameraSpeed = speedMultiplier;
	cameraSpeed *= (float) Time::GetInstance()->GetElapsedTimeSinceLastFrame();
	auto cameraRight = glm::normalize(glm::cross(myCameraFront, myCameraUp));

	const auto& inputs = Manager::InputManager::GetInstance()->GetKeyBoardState();

	if (inputs[GLFW_KEY_W])
		myNextCameraPos += cameraSpeed * myCameraFront;
	if (inputs[GLFW_KEY_S])
		myNextCameraPos -= cameraSpeed * myCameraFront;
	if (inputs[GLFW_KEY_A])
		myNextCameraPos -= cameraSpeed * cameraRight;
	if (inputs[GLFW_KEY_D])
		myNextCameraPos += cameraSpeed * cameraRight;
	if (inputs[GLFW_KEY_Q])
		myNextCameraPos.y += cameraSpeed;
	if (inputs[GLFW_KEY_E])
		myNextCameraPos.y -= cameraSpeed;
}

void Camera::OnMouseMotion(const float deltaX, const float deltaY)
{
	auto& front = myCameraFront;
	front = glm::rotate(front, deltaY, glm::cross(myCameraFront, myCameraUp));
	front = glm::rotateY(front, deltaX);
}

void Camera::Update()
{
	UpdateFromKeyboard();

	myCameraPos = myNextCameraPos;
	// UGLY UGLY UGLY UGLY UGLY UGLY UGLY UGLY UGLY UGLY
	myCameraPos -= 50.f * myCameraFront;

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
	myCameraPos += 50.f * myCameraFront;

	myNextCameraPos = myCameraPos;
}