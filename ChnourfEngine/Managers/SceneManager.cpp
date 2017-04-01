#pragma once
#include "SceneManager.h"
#include "ModelManager.h"

using namespace Manager;

SceneManager::SceneManager()
{
	glEnable(GL_DEPTH_TEST);

	myShaderManager = std::make_unique<ShaderManager>();
	myShaderManager->CreateProgram("colorShader", "Shaders\\Vertex_Shader.glsl", "Shaders\\Fragment_Shader.glsl");

	glutSetKeyRepeat(GLUT_KEY_REPEAT_ON);
	glutKeyboardFunc(KeyboardCallback);
	glutMouseFunc(MouseCallback);
	glutMotionFunc(MotionCallback);

	myModelManager = std::make_unique<ModelManager>(myShaderManager.get());

	myPointLight = DirectionalLight(glm::vec3(0.5f, -1.f, -0.5f), glm::vec3(0.6f, 0.8f, 0.8f));
}

SceneManager::~SceneManager()
{
}

void SceneManager::NotifyBeginFrame()
{
	myCurrentCamera.Update();
	myModelManager->Update();
}

void SceneManager::NotifyDisplayFrame()
{

	GLint viewPosLoc = glGetUniformLocation(myShaderManager->GetShader("colorShader"), "viewPos");
	glUniform3f(viewPosLoc, myCurrentCamera.myCameraPos.x, myCurrentCamera.myCameraPos.y, myCurrentCamera.myCameraPos.z);

	GLuint lightPos = glGetUniformLocation(myShaderManager->GetShader("colorShader"), "lightDirection");
	glUniform3f(lightPos, myPointLight.GetDirection().x, myPointLight.GetDirection().y, myPointLight.GetDirection().z);

	GLuint lightCol = glGetUniformLocation(myShaderManager->GetShader("colorShader"), "lightColor");
	glUniform3f(lightCol, myPointLight.GetIntensity().r, myPointLight.GetIntensity().g, myPointLight.GetIntensity().b);

	glm::mat4 projection;
	projection = glm::perspective(45.0f, 800.f / 600.f, 0.1f, 100.0f);
	GLuint transformLoc2 = glGetUniformLocation(myShaderManager->GetShader("colorShader"), "projection");
	glUniformMatrix4fv(transformLoc2, 1, GL_FALSE, glm::value_ptr(projection));

	auto cameraTransform = glm::lookAt(myCurrentCamera.myCameraPos, myCurrentCamera.myCameraPos + myCurrentCamera.myCameraFront, myCurrentCamera.myCameraUp);
	//glm::mat4 view;
	//// Note that we're translating the scene in the reverse direction of where we want to move
	//view = glm::translate(view, glm::vec3(0.0f, -.2f, -5.0f));
	//view = glm::rotate(view, 0.5f, glm::vec3(1.0f, 0.0f, 0.0f));
	GLuint transformLoc3 = glGetUniformLocation(myShaderManager->GetShader("colorShader"), "view");
	glUniformMatrix4fv(transformLoc3, 1, GL_FALSE, glm::value_ptr(cameraTransform));

	myModelManager->Draw(cameraTransform, myShaderManager.get());
}

void SceneManager::NotifyEndFrame()
{
	//nothing here for the moment
}

void SceneManager::NotifyReshape(int aWidth, int aHeight, int aPreviousWidth, int aPreviousHeight)
{
	//nothing here for the moment 
}

void SceneManager::KeyboardCallback(unsigned char key, int x, int y)
{
	assert(ourInstance);
	auto cameraSpeed = 0.2f;
	auto cameraRight = glm::normalize(glm::cross(ourInstance->myCurrentCamera.myCameraFront, ourInstance->myCurrentCamera.myCameraUp));
	//to do : take elapsed time into account
	switch (key)
	{
	case 'z':
		ourInstance->myCurrentCamera.myCameraPos += cameraSpeed * ourInstance->myCurrentCamera.myCameraFront;
		break;
	case 's':
		ourInstance->myCurrentCamera.myCameraPos -= cameraSpeed * ourInstance->myCurrentCamera.myCameraFront;
		break;
	case 'q':
		ourInstance->myCurrentCamera.myCameraPos -= cameraSpeed * cameraRight;
		break;
	case 'd':
		ourInstance->myCurrentCamera.myCameraPos += cameraSpeed * cameraRight;
		break;
	case 'a':
		ourInstance->myCurrentCamera.myCameraPos.y += cameraSpeed;
		break;
	case 'e':
		ourInstance->myCurrentCamera.myCameraPos.y -= cameraSpeed;
		break;
	}
}

int xOrigin = -1;
int yOrigin = -1;

void SceneManager::MouseCallback(int button, int state, int x, int y)
{
	if (button == GLUT_LEFT_BUTTON) {

		// when the button is released
		if (state == GLUT_UP)
		{
			xOrigin = -1;
			yOrigin = -1;
		}
		else
		{
			xOrigin = x;
			yOrigin = y;
		}
	}
}

void SceneManager::MotionCallback(int x, int y)
{
	// this will only be true when the left button is down
	if (xOrigin >= 0 && yOrigin >= 0) {

		// update deltaAngle
		float deltaX = ((float)(x - xOrigin)) * 0.004f;
		xOrigin = x;
		float deltaY = ((float)(y - yOrigin)) * 0.004f;
		yOrigin = y;

		auto& front = ourInstance->myCurrentCamera.myCameraFront;
		front = glm::rotate(front, deltaY, glm::cross(ourInstance->myCurrentCamera.myCameraFront, ourInstance->myCurrentCamera.myCameraUp));
		front = glm::rotateY(front, deltaX);
	}
}
