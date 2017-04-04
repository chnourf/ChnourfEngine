#pragma once

#include "../Dependencies/glew/glew.h"
#include "../Dependencies/freeglut/freeglut.h"
#include <iostream>
#include <map>

namespace Manager
{
	class ShaderManager
	{
	public:
		ShaderManager();
		~ShaderManager();

		void CreateProgram(const std::string& aShaderName, const std::string& aVertexShaderFilename, const std::string& aFragmentShaderFilename);
		const GLuint GetShader (const std::string& aShaderName) const;
		void DeleteShader(const std::string& aShaderName);

	private:
		std::string ReadShader(const std::string& aFilename);
		GLuint CreateShader(GLenum aShaderType, std::string aSource, const std::string& aShaderName);

		std::map<std::string, GLuint> myPrograms;
	};
}