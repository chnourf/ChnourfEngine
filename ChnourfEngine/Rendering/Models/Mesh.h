#pragma once
#include <vector>
#include <iostream>
#include "../../Dependencies/glew/glew.h"
#include "../../Dependencies/GLFW/glfw3.h"
#include "../VertexFormat.h"
#include "../TextureFormat.h"

namespace Manager
{
	class ShaderManager;
}

namespace Rendering
{
	class Mesh {
	public:
		std::vector<Vertex> vertices;
		std::vector<GLuint> indices;
		std::vector<TextureFormat> textures;

		Mesh(const std::vector<Vertex>& aVertices, const std::vector<GLuint>& anIndices, const std::vector<TextureFormat>& aTextures);
		~Mesh();
		void Draw(const GLuint aShader, const Manager::ShaderManager* aShaderManager);
		void DrawForShadowMap(const Manager::ShaderManager* aShaderManager);

	private:
		GLuint VAO, VBO, EBO;

		void SetupMesh();
	};
}