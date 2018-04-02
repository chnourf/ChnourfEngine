#include "ShaderManager.h" 

#include <algorithm>
#include <cassert>
#include<iostream>
#include<fstream>

using namespace Manager;

ShaderManager::ShaderManager() {}

void ShaderManager::Initialize()
{
	CreateProgram("transparentShader", "Shaders\\Vertex_Shader.glsl", "Shaders\\StandardBlinnTransparent_Shader.glsl");
	CreateProgram("colorShader", "Shaders\\Vertex_Shader.glsl", "Shaders\\StandardBlinn_Shader.glsl");
	CreateProgram("frameBufferShader", "Shaders\\FBO_Vertex_Shader.glsl", "Shaders\\FBO_Pixel_Shader.glsl");
	CreateProgram("cubemapShader", "Shaders\\Cubemap_Vertex_Shader.glsl", "Shaders\\Cubemap_Pixel_Shader.glsl");
	CreateProgram("shadowMapShader", "Shaders\\Simple_Depth_Shader.glsl", "Shaders\\Empty_Fragment_Shader.glsl");

	std::vector<std::string> seaPixelShader;
	seaPixelShader.push_back("Shaders\\Environment.h");
	seaPixelShader.push_back("Shaders\\Sea_Fragment_Shader.glsl");
	CreateProgram("seaShader", "Shaders\\Sea_Vertex_Shader.glsl", seaPixelShader);

	std::vector<std::string> skyboxPixelShader;
	skyboxPixelShader.push_back("Shaders\\Environment.h");
	skyboxPixelShader.push_back("Shaders\\Skybox_Pixel_Shader.glsl");
	CreateProgram("skyboxShader", "Shaders\\Cubemap_Vertex_Shader.glsl", skyboxPixelShader);
	CreateProgram("reflectionShader", "Shaders\\Vertex_Shader.glsl", "Shaders\\StandardBlinnReflection_Shader.glsl");


	std::vector<std::string> terrainFragmentShader;
	terrainFragmentShader.push_back("Shaders\\Environment.h");
	terrainFragmentShader.push_back("Shaders\\Terrain_Pixel_Shader.glsl");
	CreateProgram("terrainShader", "Shaders\\Terrain_Vertex_Shader.glsl", terrainFragmentShader);
	CreateProgram("shadowMapTerrainShader", "Shaders\\Terrain_Vertex_Shader_ShadowMap.glsl", "Shaders\\Empty_Fragment_Shader.glsl");

	std::vector<std::string> grassFragmentShader;
	grassFragmentShader.push_back("Shaders\\Environment.h");
	grassFragmentShader.push_back("Shaders\\Grass_Fragment_Shader.glsl");
	CreateProgram("grassShader", "Shaders\\Grass_Vertex_Shader.glsl", grassFragmentShader);

	// mandatory for UBO
	for (auto shader : GetShaders())
	{
		GLuint uniformBlockIndex = glGetUniformBlockIndex(shader.myId, "ViewConstants");
		glUniformBlockBinding(shader.myId, uniformBlockIndex, 0);
	}
}

ShaderManager::~ShaderManager()
{
	for (auto program : myPrograms)
	{
		glDeleteProgram(program.myId);
	}

	myPrograms.clear();
}

std::string ShaderManager::ReadShader(const std::string& aFilename)
{

	std::string shaderCode;
	std::ifstream file(aFilename.c_str(), std::ios::in);

	if (!file.good())
	{
		std::cout << "Can't read file " << aFilename.c_str() << std::endl;
		std::terminate();
	}

	file.seekg(0, std::ios::end);
	shaderCode.resize((unsigned int)file.tellg());
	file.seekg(0, std::ios::beg);
	file.read(&shaderCode[0], shaderCode.size());
	file.close();
	return shaderCode;
}

GLuint ShaderManager::CreateShader(GLenum aShaderType, std::string aSource, const std::string& aShaderName)
{
	int compile_result = 0;

	GLuint shader = glCreateShader(aShaderType);
	const char* shader_code_ptr = aSource.c_str();
	const int shader_code_size = (int) aSource.size();

	glShaderSource(shader, 1, &shader_code_ptr, &shader_code_size);
	glCompileShader(shader);
	glGetShaderiv(shader, GL_COMPILE_STATUS, &compile_result);

	//check for errors
	if (compile_result == GL_FALSE)
	{
		int info_log_length = 0;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &info_log_length);
		std::vector<char> shader_log(info_log_length);
		glGetShaderInfoLog(shader, info_log_length, NULL, &shader_log[0]);
		std::cout << "ERROR compiling shader: " << aShaderName.c_str() << std::endl << &shader_log[0] << std::endl;
		return 0;
	}
	return shader;
}

void ShaderManager::CreateProgramFromSource(const std::string& aShaderName, const std::string& aVertexShaderSource, const std::string& aFragmentShaderSource, const std::string* aGeometryShaderSource)
{
	GLuint vertex_shader = CreateShader(GL_VERTEX_SHADER, aVertexShaderSource, "vertex shader");
	GLuint fragment_shader = CreateShader(GL_FRAGMENT_SHADER, aFragmentShaderSource, "fragment shader");
	GLuint geometry_shader;
	if (aGeometryShaderSource)
	{
		geometry_shader = CreateShader(GL_GEOMETRY_SHADER, *aGeometryShaderSource, "geometry shader");
	}

	int link_result = 0;
	//create the program handle, attatch the shaders and link it
	GLuint program = glCreateProgram();
	glAttachShader(program, vertex_shader);
	glAttachShader(program, fragment_shader);
	if (aGeometryShaderSource)
	{
		glAttachShader(program, geometry_shader);
	}

	glLinkProgram(program);
	glGetProgramiv(program, GL_LINK_STATUS, &link_result);
	//check for link errors
	if (link_result == GL_FALSE)
	{
		int info_log_length = 0;
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &info_log_length);
		std::vector<char> program_log(info_log_length);
		glGetProgramInfoLog(program, info_log_length, NULL, &program_log[0]);
		std::cout << "Shader Loader : LINK ERROR in shader " << aShaderName.c_str() << std::endl << &program_log[0] << std::endl;
	}

	glDeleteShader(vertex_shader);
	glDeleteShader(fragment_shader);
	if (aGeometryShaderSource)
	{
		glDeleteShader(geometry_shader);
	}

	myPrograms.push_back(Program(aShaderName, program));
}

void ShaderManager::CreateProgram(const std::string& aShaderName, const std::string& aVertexShaderFilename, const std::string& aFragmentShaderFilename, const std::string* aGeometryShaderFilename)
{
	//read the shader files and save the code
	std::string vertex_shader_code = ReadShader(aVertexShaderFilename);
	std::string fragment_shader_code = ReadShader(aFragmentShaderFilename);
	if (aGeometryShaderFilename)
	{
		std::string geometry_shader_code = ReadShader(*aGeometryShaderFilename);
		CreateProgramFromSource(aShaderName, vertex_shader_code, fragment_shader_code, &geometry_shader_code);
	}
	else
	{
		CreateProgramFromSource(aShaderName, vertex_shader_code, fragment_shader_code);
	}
}

void ShaderManager::CreateProgram(const std::string& aShaderName, const std::string& aVertexShaderFilename, const std::vector<std::string>& aFragmentShaderFilename, const std::vector<std::string>* aGeometryShaderFilename)
{
	//read the shader files and save the code
	std::string vertex_shader_code;
	std::string fragment_shader_code;

	vertex_shader_code = ReadShader(aVertexShaderFilename);

	for (auto fragmentShader : aFragmentShaderFilename)
	{
		auto currentCode = ReadShader(fragmentShader);
		currentCode.erase(std::remove_if(currentCode.begin(), currentCode.end(), [](char x) {return x == 0; }), currentCode.end());
		fragment_shader_code += currentCode;
	}

	fragment_shader_code += '\0';

	std::string* geometry_shader_code = nullptr;
	std::string temp_geometry_shader_code;

	if (aGeometryShaderFilename)
	{
		for (auto geometrytShader : *aGeometryShaderFilename)
		{
			auto currentCode = ReadShader(geometrytShader);
			currentCode.erase(std::remove_if(currentCode.begin(), currentCode.end(), [](char x) {return x == 0; }), currentCode.end());
			temp_geometry_shader_code += currentCode;
		}

		temp_geometry_shader_code += '\0';
		geometry_shader_code = &temp_geometry_shader_code;
	}

	CreateProgramFromSource(aShaderName, vertex_shader_code, fragment_shader_code, geometry_shader_code);
}

const GLuint ShaderManager::GetShader(const std::string& aShaderName) const
{
	for (auto program : myPrograms)
	{
		if (program.myShaderName == aShaderName)
		{
			return program.myId;
		}
	}
	assert(false);
	return 0;
}

void ShaderManager::DeleteShader(const std::string& aShaderName)
{
	for (auto it = myPrograms.begin(); it != myPrograms.end(); ++it)
	{
		auto program = *it;
		if (program.myShaderName == aShaderName)
		{
			glDeleteProgram(program.myId);
			std::iter_swap(it, myPrograms.end() - 1);
			myPrograms.erase(myPrograms.end() - 1, myPrograms.end());
			return;
		}
	}

	assert(false);
}

void Manager::ShaderManager::CreateComputeProgram(const std::string& aShaderName, const std::string& aComputeShaderFilename)
{
	//read the shader files and save the code
	std::string compute_shader_code = ReadShader(aComputeShaderFilename);
	CreateComputeProgramFromSource(aShaderName, compute_shader_code);
}

void Manager::ShaderManager::CreateComputeProgramFromSource(const std::string& aShaderName, const std::string& aComputeShaderSource)
{
	GLuint compute_shader = CreateShader(GL_COMPUTE_SHADER, aComputeShaderSource, aShaderName);

	int link_result = 0;
	//create the program handle, attach the shaders and link it
	GLuint program = glCreateProgram();
	glAttachShader(program, compute_shader);

	glLinkProgram(program);
	glGetProgramiv(program, GL_LINK_STATUS, &link_result);
	//check for link errors
	if (link_result == GL_FALSE)
	{
		int info_log_length = 0;
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &info_log_length);
		std::vector<char> program_log(info_log_length);
		glGetProgramInfoLog(program, info_log_length, NULL, &program_log[0]);
		std::cout << "Shader Loader : LINK ERROR in shader " << aShaderName.c_str() << std::endl << &program_log[0] << std::endl;
	}

	glDeleteShader(compute_shader);

	myPrograms.push_back(Program(aShaderName, program));
}
