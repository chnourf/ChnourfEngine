#pragma once
#include "SceneManager.h"
#include "ModelManager.h"
#include "../Core/Math.h"
#include "../WorldGenerator/TerrainManager.h"
#include "../Core/Vector.h"
#include "../Core/Intersection.h"
#include "../Core/Time.h"
#include <ctime>
#include "../Dependencies/GLFW/glfw3.h"
#include "../Dependencies/glew/glew.h"
#include "../Core/Init/GLFWwrapper.h"
#include "../Dependencies/imgui/imgui.h"


using namespace Manager;

SceneManager::SceneManager()
{
	myShaderManager = std::make_unique<ShaderManager>();
	myModelManager = std::make_unique<ModelManager>();

	auto dir = glm::vec3(1.f, -.5f, 1.f);
	dir = glm::normalize(dir);
	myDirectionalLight = DirectionalLight(dir, glm::vec3(3.f, 3.f, 2.8f));
}

SceneManager::~SceneManager()
{
	glDeleteFramebuffers(1, &fbo);
}

float testFloat = 3.f;

void SceneManager::Initialize()
{
	myShaderManager->Initialize();

	int windowWidth, windowHeight;
	Core::Init::GLFWwrapper::GetWindowWidthAndHeight(windowWidth, windowHeight);
	myCurrentCamera.Initialize(windowWidth, windowHeight);
	//nextCameraPosition = myCurrentCamera.myCameraPos;

	// Setting up main framebuffer ----------------------------------------------------------------------------------------------
	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);

	glGenTextures(1, &textureColorBuffer);
	glBindTexture(GL_TEXTURE_2D, textureColorBuffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, windowWidth, windowHeight, 0, GL_RGBA, GL_FLOAT, nullptr); // for HDR
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureColorBuffer, 0);
	
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, windowWidth, windowHeight, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, nullptr);
	GLuint rbo;
	glGenRenderbuffers(1, &rbo);
	glBindRenderbuffer(GL_RENDERBUFFER, rbo);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, windowWidth, windowHeight);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
		assert(0);
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// Setting up the quad where to display the fbo ----------------------------------------------------------------------------
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

	// setting up view UBO --------------------------------------------------------------------------------------------------------
	glGenBuffers(1, &myViewConstantUbo);
	glBindBuffer(GL_UNIFORM_BUFFER, myViewConstantUbo);
	glBufferData(GL_UNIFORM_BUFFER, 2 * sizeof(glm::mat4), NULL, GL_STATIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
	glBindBufferRange(GL_UNIFORM_BUFFER, 0, myViewConstantUbo, 0, 2 * sizeof(glm::mat4));

	// setting up Shadow Map --------------------------------------------------------------------------------------------------------
	glGenFramebuffers(1, &shadowMapFBO);

	glGenTextures(1, &shadowMapTexture);
	glBindTexture(GL_TEXTURE_2D, shadowMapTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, shadowMapResolution, shadowMapResolution, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
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

	glGenTextures(1, &myNoiseTexture);
	glBindTexture(GL_TEXTURE_2D, myNoiseTexture);
	int width, height;
	unsigned char* image = SOIL_load_image("Data/noise.jpg", &width, &height, 0, SOIL_LOAD_RGB);
	assert(image);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
	glGenerateMipmap(GL_TEXTURE_2D);
	SOIL_free_image_data(image);
	glBindTexture(GL_TEXTURE_2D, 0);

	//myModelManager->FillScene(myShaderManager.get());
}

void SceneManager::NotifyBeginFrame()
{
	myCurrentCamera.myCameraPos;

	Time::GetInstance()->Update();
	myCurrentCamera.Update();
	TerrainManager::GetInstance()->Update(vec3f(myCurrentCamera.myCameraPos.x, myCurrentCamera.myCameraPos.y, myCurrentCamera.myCameraPos.z));
	myModelManager->Update();
}

//ugly
float atmospheric_depth(glm::vec3 position, glm::vec3 dir) {
	double a = dot(dir, dir);
	double b = 2.0*dot(dir, position);
	double c = dot(position, position) - 1.0;
	double det = b*b - 4.0*a*c;
	double detSqrt = sqrt(det);
	double q = (-b - detSqrt) / 2.0;
	double t1 = c / q;
	return (float) t1;
}

float angle = 0.f;
void SceneManager::NotifyDisplayFrame()
{
	myModelManager->ResetCulling();

	//float multiplier = 0.1f;
	//if (lightDir.y > 0.01f)
	//{
	//	multiplier = 10.f;
	//}
	ImGui::SliderFloat("sun angle", &angle, -1.f, 1.f);
	myDirectionalLight.SetDirection(glm::rotateX(glm::normalize(glm::vec3(1.f, -.5f, 1.f)), (float)M_PI * angle));
	auto& lightDir = myDirectionalLight.GetDirection();

	ImGui::Text("To sun direction : x %f, y %f, z%f", -lightDir.x, -lightDir.y, -lightDir.z);

	glm::vec3 Kr = glm::vec3(5.5e-6f, 13.0e-6f, 22.4e-6f);
	glm::vec3 eye_position = glm::vec3(0.0f, 1.f-13.f/6400.f, 0.0f);
	float eye_depth = atmospheric_depth(eye_position, -glm::normalize(lightDir));
	glm::vec3 sunColor = lightDir.y > 0.0f ? glm::vec3(0.f, 0.2f, 0.5f) : (3.f*(exp(-eye_depth*Kr*6e6f)));
	myDirectionalLight.SetIntensity(sunColor);

	// setting all uniform variables, could be optimized
	for (auto shader : myShaderManager->GetShaders())
	{
		auto programId = shader.myId;
		glUseProgram(programId);

		GLint viewPosLoc = glGetUniformLocation(programId, "viewPos");
		glUniform3f(viewPosLoc, myCurrentCamera.myCameraPos.x, myCurrentCamera.myCameraPos.y, myCurrentCamera.myCameraPos.z);

		GLuint lightDirectionId = glGetUniformLocation(programId, "lightDirection");
		glUniform3f(lightDirectionId, lightDir.x, lightDir.y, lightDir.z);

		GLuint lightCol = glGetUniformLocation(programId, "lightColor");
		glUniform3f(lightCol, myDirectionalLight.GetIntensity().r, myDirectionalLight.GetIntensity().g, myDirectionalLight.GetIntensity().b);

		GLuint lightSpaceMatrixLocation = glGetUniformLocation(programId, "lightSpaceMatrix");
		glUniformMatrix4fv(lightSpaceMatrixLocation, 1, GL_FALSE, glm::value_ptr(myLightSpaceMatrix));

		GLuint shadowMapResolutionId = glGetUniformLocation(programId, "invShadowMapResolution");
		glUniform1f(shadowMapResolutionId, float(1.f/shadowMapResolution));

		glUniform1i(glGetUniformLocation(programId, "noise"), 6);
		glActiveTexture(GL_TEXTURE6);
		glBindTexture(GL_TEXTURE_2D, myNoiseTexture);

		glUniform1i(glGetUniformLocation(programId, "shadowMap"), 7);
		glActiveTexture(GL_TEXTURE7);
		glBindTexture(GL_TEXTURE_2D, shadowMapTexture);
	}

	// Shadow Map Pass ----------------------------------------------------------------------------------------------------------------------
	glViewport(0, 0, shadowMapResolution, shadowMapResolution);
	glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
	glClear(GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_DEPTH_CLAMP); // doesn't seem to work

	glm::mat4 lightProjection = glm::ortho(-300.0f, 300.0f, -300.0f, 300.0f, -1000.f, 1000.f);
	glm::mat4 lightView = glm::lookAt(myCurrentCamera.myCameraPos - 500.f * myDirectionalLight.GetDirection(), myCurrentCamera.myCameraPos,	glm::vec3(0.0f, 1.0f, 0.0f));
	myLightSpaceMatrix = lightProjection * lightView;

	glBindBuffer(GL_UNIFORM_BUFFER, myViewConstantUbo);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(lightProjection));
	glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(lightView));
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	auto shadowMapProgram = myShaderManager->GetShader("shadowMapShader");
	glUseProgram(shadowMapProgram);
	GLuint lightSpaceMatrixLocation = glGetUniformLocation(shadowMapProgram, "lightSpaceMatrix");
	glUniformMatrix4fv(lightSpaceMatrixLocation, 1, GL_FALSE, glm::value_ptr(myLightSpaceMatrix));

	myModelManager->DrawShadowMap(myShaderManager.get());

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glDisable(GL_DEPTH_CLAMP);

	// First color pass ----------------------------------------------------------------------------------------------------------------------
	auto cameraTransform = glm::lookAt(myCurrentCamera.myCameraPos, myCurrentCamera.myCameraPos + myCurrentCamera.myCameraFront, myCurrentCamera.myCameraUp);
	glBindBuffer(GL_UNIFORM_BUFFER, myViewConstantUbo);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(myCurrentCamera.GetProjectionMatrix()));
	glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(cameraTransform));
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	int width, height;
	Core::Init::GLFWwrapper::GetWindowWidthAndHeight(width, height);
	glViewport(0, 0, width, height);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);

	myModelManager->CullScene(myCurrentCamera);

	// for Terrain
	auto terrainShaderID = myShaderManager->GetShader("terrainShader");
	glUseProgram(terrainShaderID);

	GLuint tileSize = glGetUniformLocation(terrainShaderID, "tileSize");
	glUniform1i(tileSize, TerrainManager::GetInstance()->GetTileSize());

	GLuint tileResolution = glGetUniformLocation(terrainShaderID, "resolution");
	glUniform1f(tileResolution, TerrainManager::GetInstance()->GetResolution());

	auto terrainShadowShaderID = myShaderManager->GetShader("shadowMapTerrainShader");
	glUseProgram(terrainShadowShaderID);

	tileSize = glGetUniformLocation(terrainShadowShaderID, "tileSize");
	glUniform1i(tileSize, TerrainManager::GetInstance()->GetTileSize());

	tileResolution = glGetUniformLocation(terrainShadowShaderID, "resolution");
	glUniform1f(tileResolution, TerrainManager::GetInstance()->GetResolution());
	
	myModelManager->Draw(cameraTransform, myShaderManager.get());

	// rendering Skybox at last
	glDepthMask(GL_FALSE);
	glm::mat4 view = glm::mat4(glm::mat3(cameraTransform));
	mySkybox.Render(myShaderManager->GetShader("skyboxShader"), view);
	glDepthMask(GL_TRUE);

	// Second pass -------------------------------------------------------------------------------------------------------------------------------------------
	glBindFramebuffer(GL_FRAMEBUFFER, 0); // back to default
	//clear should be done in InitGLUT
	glDisable(GL_FRAMEBUFFER_SRGB);

	auto postEffectProgram = myShaderManager->GetShader("frameBufferShader");
	glUseProgram(postEffectProgram);
	GLint exposureID = glGetUniformLocation(postEffectProgram, "exposure");
	glUniform1f(exposureID, glm::smoothstep(-0.1f, 0.1f,myDirectionalLight.GetDirection().y)*4 + 4);
	glBindVertexArray(framebufferQuadVao);
	glActiveTexture(GL_TEXTURE0);
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
	int width, height;
	Core::Init::GLFWwrapper::GetWindowWidthAndHeight(width, height);

	glm::mat4 projection;
	projection = glm::perspective(45.0f, (float)width / (float)height, 0.1f, 3000.0f);
	glBindBuffer(GL_UNIFORM_BUFFER, myViewConstantUbo);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(projection));
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}