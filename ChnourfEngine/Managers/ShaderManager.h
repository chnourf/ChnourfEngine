#pragma once

#include "../Dependencies/glew/glew.h"
#include "../Dependencies/freeglut/freeglut.h"
#include <iostream>
#include <vector>

struct Program
{
	Program(){}

	Program(const std::string& aName, GLuint anId) :
		myShaderName(aName),
		myId(anId)
	{}

	std::string myShaderName;
	GLuint myId;

	//add rendering queue here;
};

namespace Manager
{
	class ShaderManager
	{
	public:
		ShaderManager();
		~ShaderManager();

		void CreateProgram(const std::string& aShaderName, const std::string& aVertexShaderFilename, const std::string& aFragmentShaderFilename);
		void CreateProgram(const std::string& aShaderName, const std::vector<std::string>& aVertexShaderFilename, const std::vector<std::string>& aFragmentShaderFilename);
		void Initialize();
		const GLuint GetShader (const std::string& aShaderName) const;
		const std::vector<Program> GetShaders() const { return myPrograms; }
		void DeleteShader(const std::string& aShaderName);

	private:
		std::string ReadShader(const std::string& aFilename);
		void CreateProgramFromSource(const std::string& aShaderName, const std::string& aVertexShaderSource, const std::string& aFragmentShaderSource);
		GLuint CreateShader(GLenum aShaderType, std::string aSource, const std::string& aShaderName);

		std::vector<Program> myPrograms;


	};
}