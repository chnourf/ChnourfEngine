#pragma once
#include "SceneManager.h"
#include "ModelManager.h"
#include "../Core/Math.h"
#include "../WorldGenerator/TerrainManager.h"
#include "../Core/Vector.h"

using namespace Manager;

SceneManager::SceneManager()
{
	myShaderManager = std::make_unique<ShaderManager>();
	myShaderManager->CreateProgram("transparentShader", "Shaders\\Vertex_Shader.glsl", "Shaders\\StandardBlinnTransparent_Shader.glsl");
	myShaderManager->CreateProgram("colorShader", "Shaders\\Vertex_Shader.glsl", "Shaders\\StandardBlinn_Shader.glsl");
	myShaderManager->CreateProgram("frameBufferShader", "Shaders\\FBO_Vertex_Shader.glsl", "Shaders\\FBO_Pixel_Shader.glsl");
	myShaderManager->CreateProgram("cubemapShader", "Shaders\\Cubemap_Vertex_Shader.glsl", "Shaders\\Cubemap_Pixel_Shader.glsl");
	myShaderManager->CreateProgram("reflectionShader", "Shaders\\Vertex_Shader.glsl", "Shaders\\StandardBlinnReflection_Shader.glsl");
	myShaderManager->CreateProgram("terrainShader", "Shaders\\Terrain_Vertex_Shader.glsl", "Shaders\\Terrain_Pixel_Shader.glsl");
	myShaderManager->CreateProgram("shadowMapShader", "Shaders\\Simple_Depth_Shader.glsl", "Shaders\\Empty_Fragment_Shader.glsl");

	// mandatory for UBO
	for (auto shader : myShaderManager->GetShaders())
	{
		GLuint uniformBlockIndex = glGetUniformBlockIndex(shader.myId, "ViewConstants");
		glUniformBlockBinding(shader.myId, uniformBlockIndex, 0);
	}

	glutSetKeyRepeat(GLUT_KEY_REPEAT_ON);
	glutKeyboardFunc(KeyboardCallback);
	glutMouseFunc(MouseCallback);
	glutMotionFunc(MotionCallback);

	myModelManager = std::make_unique<ModelManager>();

	myTerrainManager = std::make_unique<TerrainManager>();

	myDirectionalLight = DirectionalLight(glm::vec3(1.f, -1.f, 1.f), glm::vec3(1.f, 1.f, 1.f));

	mySkybox.LoadTextures("Data\\Skybox");
}

SceneManager::~SceneManager()
{
	glDeleteFramebuffers(1, &fbo);
}

void SceneManager::Initialize(const Core::WindowInfo& aWindow)
{
	myWindowHeigth = aWindow.height;
	myWindowWidth = aWindow.width;

	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);

	glGenTextures(1, &textureColorBuffer);
	glBindTexture(GL_TEXTURE_2D, textureColorBuffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, myWindowWidth, myWindowHeigth, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureColorBuffer, 0);
	
	// if we don't need to sample the depth a render buffer object would be better. But I think we need it for water ?
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, myWindowWidth, myWindowHeigth, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, nullptr);
	GLuint rbo;
	glGenRenderbuffers(1, &rbo);
	glBindRenderbuffer(GL_RENDERBUFFER, rbo);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, myWindowWidth, myWindowHeigth);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
		assert(0);
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// Vertex attributes for a quad that fills the entire screen in Normalized Device Coordinates.
	GLfloat quadVertices[] = {   
		// Positions   // TexCoords
		-1.0f,  1.0f,  0.0f, 1.0f,
		-1.0f, -1.0f,  0.0f, 0.0f,
		1.0f, -1.0f,  1.0f, 0.0f,

		-1.0f,  1.0f,  0.0f, 1.0f,
		1.0f, -1.0f,  1.0f, 0.0f,
		1.0f,  1.0f,  1.0f, 1.0f
	};

	glGenVertexArrays(1, &framebufferQuadVao);
	glGenBuffers(1, &framebufferQuadVbo);
	glBindVertexArray(framebufferQuadVao);
	glBindBuffer(GL_ARRAY_BUFFER, framebufferQuadVbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (GLvoid*)(2 * sizeof(GLfloat)));
	glBindVertexArray(0);

	// setting up view UBO
	glGenBuffers(1, &myViewConstantUbo);
	glBindBuffer(GL_UNIFORM_BUFFER, myViewConstantUbo);
	glBufferData(GL_UNIFORM_BUFFER, 2 * sizeof(glm::mat4), NULL, GL_STATIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
	glBindBufferRange(GL_UNIFORM_BUFFER, 0, myViewConstantUbo, 0, 2 * sizeof(glm::mat4));

	glm::mat4 projection;
	projection = glm::perspective(45.0f, (float)myWindowWidth / (float)myWindowHeigth, 0.1f, 100.0f);
	glBindBuffer(GL_UNIFORM_BUFFER, myViewConstantUbo);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(projection));
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	// setting up Shadow Map
	glGenFramebuffers(1, &shadowMapFBO);

	glGenTextures(1, &shadowMapTexture);
	glBindTexture(GL_TEXTURE_2D, shadowMapTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, shadowMapResolution, shadowMapResolution, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	GLfloat borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
	glBindTexture(GL_TEXTURE_2D, 0);

	glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadowMapTexture, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	//myModelManager->FillScene(myShaderManager.get());
}

void SceneManager::NotifyBeginFrame()
{
	myCurrentCamera.Update();
	myTerrainManager->Update(vec3f(myCurrentCamera.myCameraPos.x, myCurrentCamera.myCameraPos.y, myCurrentCamera.myCameraPos.z));
	myModelManager->Update();
}

void SceneManager::NotifyDisplayFrame()
{
	//if (myDirectionalLight.GetDirection().y < 0.01f)
	//{
	//	myDirectionalLight.SetDirection(glm::rotateX(myDirectionalLight.GetDirection(), (float)M_PI+0.1f));
	//}
	myDirectionalLight.SetDirection(glm::rotateX(myDirectionalLight.GetDirection(), .003f));

	auto cameraTransform = glm::lookAt(myCurrentCamera.myCameraPos, myCurrentCamera.myCameraPos + myCurrentCamera.myCameraFront, myCurrentCamera.myCameraUp);
	glBindBuffer(GL_UNIFORM_BUFFER, myViewConstantUbo);
	glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(cameraTransform));
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	// Shadow Map Pass
	glViewport(0, 0, shadowMapResolution, shadowMapResolution);
	glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
	glClear(GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	auto shadowMapProgram = myShaderManager->GetShader("shadowMapShader");
	glUseProgram(shadowMapProgram);

	GLfloat near_plane = 100.0f, far_plane = 800.f;
	glm::mat4 lightProjection = glm::ortho(-50.0f, 50.0f, -50.0f, 50.0f, near_plane, far_plane);
	glm::mat4 lightView = glm::lookAt(myCurrentCamera.myCameraPos - 200.f * myDirectionalLight.GetDirection(), myCurrentCamera.myCameraPos,	glm::vec3(0.0f, 1.0f, 0.0f));
	myLightSpaceMatrix = lightProjection * lightView;
	GLuint lightSpaceMatrixLocation = glGetUniformLocation(shadowMapProgram, "lightSpaceMatrix");
	glUniformMatrix4fv(lightSpaceMatrixLocation, 1, GL_FALSE, glm::value_ptr(myLightSpaceMatrix));

	myModelManager->DrawShadowMap(shadowMapProgram);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);


	// First color pass ----------------------------------------------------------------------------------------------------------------------
	glViewport(0, 0, myWindowWidth, myWindowHeigth);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);

	

	// setting all uniform variables, could be optimized
	for (auto shader : myShaderManager->GetShaders())
	{
		auto programId = shader.myId;
		glUseProgram(programId);

		GLint viewPosLoc = glGetUniformLocation(programId, "viewPos");
		glUniform3f(viewPosLoc, myCurrentCamera.myCameraPos.x, myCurrentCamera.myCameraPos.y, myCurrentCamera.myCameraPos.z);

		GLuint lightPos = glGetUniformLocation(programId, "lightDirection");
		glUniform3f(lightPos, myDirectionalLight.GetDirection().x, myDirectionalLight.GetDirection().y, myDirectionalLight.GetDirection().z);

		GLuint lightCol = glGetUniformLocation(programId, "lightColor");
		glUniform3f(lightCol, myDirectionalLight.GetIntensity().r, myDirectionalLight.GetIntensity().g, myDirectionalLight.GetIntensity().b);

		GLuint lightSpaceMatrixLocation = glGetUniformLocation(programId, "lightSpaceMatrix");
		glUniformMatrix4fv(lightSpaceMatrixLocation, 1, GL_FALSE, glm::value_ptr(myLightSpaceMatrix));

		glUniform1i(glGetUniformLocation(programId, "shadowMap"), 3);
		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, shadowMapTexture);


		glBindTexture(GL_TEXTURE_CUBE_MAP, mySkybox.GetTexture());
	}
	
	myModelManager->Draw(cameraTransform, myShaderManager.get());

	// rendering Skybox at last
	glDepthMask(GL_FALSE);
	glm::mat4 view = glm::mat4(glm::mat3(cameraTransform));
	mySkybox.Render(myShaderManager->GetShader("cubemapShader"), view);
	glDepthMask(GL_TRUE);

	// Second pass -------------------------------------------------------------------------------------------------------------------------------------------
	glBindFramebuffer(GL_FRAMEBUFFER, 0); // back to default
	//clear should be done in InitGLUT
	glDisable(GL_FRAMEBUFFER_SRGB);

	glUseProgram(myShaderManager->GetShader("frameBufferShader"));
	glBindVertexArray(framebufferQuadVao);
	glDisable(GL_DEPTH_TEST);
	glBindTexture(GL_TEXTURE_2D, textureColorBuffer);
	//glBindTexture(GL_TEXTURE_2D, shadowMapTexture);

	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);
}

void SceneManager::NotifyEndFrame()
{
	//nothing here for the moment
}

void SceneManager::NotifyReshape(int aWidth, int aHeight, int aPreviousWidth, int aPreviousHeight)
{
	myWindowWidth = aWidth;
	myWindowHeigth = aHeight;

	glm::mat4 projection;
	projection = glm::perspective(45.0f, (float)myWindowWidth / (float)myWindowHeigth, 0.1f, 3000.0f);
	glBindBuffer(GL_UNIFORM_BUFFER, myViewConstantUbo);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(projection));
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void SceneManager::KeyboardCallback(unsigned char key, int x, int y)
{
	assert(ourInstance);
	auto cameraSpeed = 2.f;
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
