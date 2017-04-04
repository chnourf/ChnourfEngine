#include "ShaderManager.h" 
#include<iostream>
#include<fstream>
#include<vector>

using namespace Manager;

ShaderManager::ShaderManager() {}

ShaderManager::~ShaderManager()
{
	for (auto program : myPrograms)
	{
		glDeleteProgram(program.second);
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

void ShaderManager::CreateProgram(const std::string& aShaderName, const std::string& aVertexShaderFilename, const std::string& aFragmentShaderFilename)
{
	//read the shader files and save the code
	std::string vertex_shader_code = ReadShader(aVertexShaderFilename);
	std::string fragment_shader_code = ReadShader(aFragmentShaderFilename);

	GLuint vertex_shader = CreateShader(GL_VERTEX_SHADER, vertex_shader_code, "vertex shader");
	GLuint fragment_shader = CreateShader(GL_FRAGMENT_SHADER, fragment_shader_code, "fragment shader");

	int link_result = 0;
	//create the program handle, attatch the shaders and link it
	GLuint program = glCreateProgram();
	glAttachShader(program, vertex_shader);
	glAttachShader(program, fragment_shader);

	glLinkProgram(program);
	glGetProgramiv(program, GL_LINK_STATUS, &link_result);
	//check for link errors
	if (link_result == GL_FALSE)
	{

		int info_log_length = 0;
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &info_log_length);
		std::vector<char> program_log(info_log_length);
		glGetProgramInfoLog(program, info_log_length, NULL, &program_log[0]);
		std::cout << "Shader Loader : LINK ERROR" << std::endl << &program_log[0] << std::endl;
	}

	myPrograms.insert(std::pair<std::string, GLuint> (aShaderName, program));
}

const GLuint ShaderManager::GetShader(const std::string& aShaderName) const
{
	return myPrograms.at(aShaderName);
}

void ShaderManager::DeleteShader(const std::string& aShaderName)
{
	auto program = myPrograms.at(aShaderName);
	glDeleteProgram(program);
	myPrograms.erase(aShaderName);
}