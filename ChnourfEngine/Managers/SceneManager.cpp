#pragma once
#include "SceneManager.h"
#include "ModelManager.h"

using namespace Manager;

SceneManager::SceneManager()
{
	myShaderManager = std::make_unique<ShaderManager>();
	myShaderManager->CreateProgram("transparentShader", "Shaders\\Vertex_Shader.glsl", "Shaders\\StandardBlinnTransparent_Shader.glsl");
	myShaderManager->CreateProgram("colorShader", "Shaders\\Vertex_Shader.glsl", "Shaders\\StandardBlinn_Shader.glsl");
	myShaderManager->CreateProgram("frameBufferShader", "Shaders\\FBO_Vertex_Shader.glsl", "Shaders\\FBO_Pixel_Shader.glsl");
	myShaderManager->CreateProgram("cubemapShader", "Shaders\\Cubemap_Vertex_Shader.glsl", "Shaders\\Cubemap_Pixel_Shader.glsl");
	myShaderManager->CreateProgram("reflectionShader", "Shaders\\Vertex_Shader.glsl", "Shaders\\StandardBlinnReflection_Shader.glsl");

	glutSetKeyRepeat(GLUT_KEY_REPEAT_ON);
	glutKeyboardFunc(KeyboardCallback);
	glutMouseFunc(MouseCallback);
	glutMotionFunc(MotionCallback);

	myModelManager = std::make_unique<ModelManager>();

	myPointLight = DirectionalLight(glm::vec3(0.5f, -1.f, -0.5f), glm::vec3(0.6f, 0.8f, 0.8f));

	mySkybox.LoadTextures("Data\\Skybox");
}

SceneManager::~SceneManager()
{
	glDeleteFramebuffers(1, &fbo);
}

void SceneManager::Initialize()
{
	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);

	glGenTextures(1, &textureColorBuffer);
	glBindTexture(GL_TEXTURE_2D, textureColorBuffer);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 800, 600, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureColorBuffer, 0);


	// if we don't need to sample the depth a render buffer object would be better. But I think we need it for water ?
	glTexImage2D(
		GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, 800, 600, 0,
		GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, NULL
	);
	GLuint rbo;
	glGenRenderbuffers(1, &rbo);
	glBindRenderbuffer(GL_RENDERBUFFER, rbo);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, 800, 600);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;

	myModelManager->FillScene(myShaderManager.get());
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	GLfloat quadVertices[] = {   // Vertex attributes for a quad that fills the entire screen in Normalized Device Coordinates.
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
}

void SceneManager::NotifyBeginFrame()
{
	myCurrentCamera.Update();
	myModelManager->Update();
}

void SceneManager::NotifyDisplayFrame()
{
	// First pass ----------------------------------------------------------------------------------------------------------------------
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);

	//auto currentShader = myShaderManager->GetShader("colorShader");
	auto cameraTransform = glm::lookAt(myCurrentCamera.myCameraPos, myCurrentCamera.myCameraPos + myCurrentCamera.myCameraFront, myCurrentCamera.myCameraUp);

	for (auto shader : myShaderManager->GetShaders())
	{
		auto programId = shader.myId;
		glUseProgram(programId);

		GLint viewPosLoc = glGetUniformLocation(programId, "viewPos");
		glUniform3f(viewPosLoc, myCurrentCamera.myCameraPos.x, myCurrentCamera.myCameraPos.y, myCurrentCamera.myCameraPos.z);

		GLuint lightPos = glGetUniformLocation(programId, "lightDirection");
		glUniform3f(lightPos, myPointLight.GetDirection().x, myPointLight.GetDirection().y, myPointLight.GetDirection().z);

		GLuint lightCol = glGetUniformLocation(programId, "lightColor");
		glUniform3f(lightCol, myPointLight.GetIntensity().r, myPointLight.GetIntensity().g, myPointLight.GetIntensity().b);

		glm::mat4 projection;
		projection = glm::perspective(45.0f, 800.f / 600.f, 0.1f, 100.0f);
		GLuint transformLoc2 = glGetUniformLocation(programId, "projection");
		glUniformMatrix4fv(transformLoc2, 1, GL_FALSE, glm::value_ptr(projection));

		GLuint transformLoc3 = glGetUniformLocation(programId, "view");
		glUniformMatrix4fv(transformLoc3, 1, GL_FALSE, glm::value_ptr(cameraTransform));

		glBindTexture(GL_TEXTURE_CUBE_MAP, mySkybox.GetTexture());
	}


	myModelManager->Draw(cameraTransform, myShaderManager.get());

	glDepthMask(GL_FALSE);
	glm::mat4 view = glm::mat4(glm::mat3(cameraTransform));
	mySkybox.Render(myShaderManager->GetShader("cubemapShader"), view);
	glDepthMask(GL_TRUE);

	// Second pass -------------------------------------------------------------------------------------------------------------------------------------------
	glBindFramebuffer(GL_FRAMEBUFFER, 0); // back to default
	//clear should be done in InitGLUT

	glUseProgram(myShaderManager->GetShader("frameBufferShader"));
	glBindVertexArray(framebufferQuadVao);
	glDisable(GL_DEPTH_TEST);
	glBindTexture(GL_TEXTURE_2D, textureColorBuffer);

	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);
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
