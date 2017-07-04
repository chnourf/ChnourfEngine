#include "Mesh.h"

#include "../../Managers/SceneManager.h"

namespace Rendering
{
	Mesh::Mesh(const std::vector<Vertex>& aVertices, const std::vector<GLuint>& anIndices, const std::vector<TextureFormat>& aTextures)
	{
		vertices = aVertices;
		indices = anIndices;
		textures = aTextures;

		SetupMesh();
	}

	Mesh::~Mesh()
	{
		glDeleteVertexArrays(1, &VAO);
		glDeleteBuffers(1, &VBO);
	}

	void Mesh::SetupMesh()
	{
		glGenVertexArrays(1, &VAO);
		glGenBuffers(1, &VBO);
		glGenBuffers(1, &EBO);

		glBindVertexArray(VAO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);

		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex),
			&vertices[0], GL_STATIC_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint),
			&indices[0], GL_STATIC_DRAW);

		// Vertex Positions
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
			(GLvoid*)0);
		// Vertex Normals
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
			(GLvoid*)offsetof(Vertex, normal));
		// Vertex Texture Coords
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
			(GLvoid*)offsetof(Vertex, uv));

		glBindVertexArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	}

	void Mesh::Draw(const GLuint aShader, const Manager::ShaderManager* aShaderManager)
	{
		GLuint diffuseNr = 1;
		GLuint specularNr = 1;
		//ugly, this will be done for each frame and object !
		for (GLuint i = 0; i < textures.size(); i++)
		{
			glActiveTexture(GL_TEXTURE0 + i); // Activate proper texture unit before binding
											  // Retrieve texture number (the N in diffuse_textureN)
			std::string number;
			auto name = textures[i].myType;
			if (name == TextureType::diffuse)
				diffuseNr++; // Transfer GLuint to stream
			else if (name == TextureType::specular)
				specularNr++; // Transfer GLuint to stream

			glUniform1f(glGetUniformLocation(aShader, ("material." + name + number).c_str()), i);
			glBindTexture(GL_TEXTURE_2D, textures[i].myId);
		}
		glActiveTexture(GL_TEXTURE0);

		//glActiveTexture(GL_TEXTURE0);


		// Draw mesh
		glBindVertexArray(VAO);
		glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);
	}

	void Mesh::DrawForShadowMap(const Manager::ShaderManager* aShaderManager)
	{
		glBindVertexArray(VAO);
		glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);
	}
}