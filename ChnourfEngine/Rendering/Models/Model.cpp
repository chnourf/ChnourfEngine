#include "Model.h"

namespace Rendering
{
	namespace Models
	{
		Model::Model()
		{

		}

		Model::Model(const glm::mat4& aTransform)
		{
			myTransform = aTransform;
		}

		Model::Model(const glm::mat4& aTransform, const Material& aMaterial)
		{
			myTransform = aTransform;
			myMaterial = aMaterial;
		}

		Model::~Model()
		{
			Destroy();
		}

		void Model::Create()
		{
			GLuint texture1, texture2;
			myTextures.push_back(texture1);
			myTextures.push_back(texture2);

			GLuint vao;
			GLuint vbo;

			glGenVertexArrays(1, &vao);
			glBindVertexArray(vao);

			CreateTexture(myTextures[0], "Data/container2.png");
			CreateTexture(myTextures[1], "Data/container2_specular.png");

			GLfloat vertices[] = {
				// Positions           // Normals           // Texture Coords
				-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,
				0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 0.0f,
				0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
				0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
				-0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 1.0f,
				-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,

				-0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   0.0f, 0.0f,
				0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   1.0f, 0.0f,
				0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   1.0f, 1.0f,
				0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   1.0f, 1.0f,
				-0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   0.0f, 1.0f,
				-0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   0.0f, 0.0f,

				-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
				-0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
				-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
				-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
				-0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
				-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,

				0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
				0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
				0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
				0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
				0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
				0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,

				-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,
				0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 1.0f,
				0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
				0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
				-0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 0.0f,
				-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,

				-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f,
				0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 1.0f,
				0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
				0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
				-0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 0.0f,
				-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f
			};

			glGenBuffers(1, &vbo);
			glBindBuffer(GL_ARRAY_BUFFER, vbo);
			glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), &vertices[0], GL_STATIC_DRAW);
			glEnableVertexAttribArray(0);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)0);
			glEnableVertexAttribArray(1);
			glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));
			glEnableVertexAttribArray(2);
			glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)(6 * sizeof(GLfloat)));
			glBindVertexArray(0);

			//here we assign the values
			this->vao = vao;
			this->vbos.push_back(vbo);
		}

		void Model::CreateTexture(GLuint& aTextureID, const std::string& aPath)
		{
			glGenTextures(1, &aTextureID);
			glBindTexture(GL_TEXTURE_2D, aTextureID);
			int width, height;
			unsigned char* image = SOIL_load_image(aPath.c_str(), &width, &height, 0, SOIL_LOAD_RGB);
			assert(image);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
			glGenerateMipmap(GL_TEXTURE_2D);
			SOIL_free_image_data(image);
			glBindTexture(GL_TEXTURE_2D, 0);
		}

		void Model::Draw()
		{
			GLuint transformLoc1 = glGetUniformLocation(program, "model");
			glUniformMatrix4fv(transformLoc1, 1, GL_FALSE, glm::value_ptr(myTransform));

			glm::mat4 trans;
			trans = glm::rotate(trans, glutGet(GLUT_ELAPSED_TIME) * 1.0f / 1000.0f, glm::vec3(0.0, 0.0, 1.0));
			trans = glm::translate(trans, glm::vec3(0.5f, -0.5f, 0.0f));

			GLuint transformLoc = glGetUniformLocation(program, "transform");
			glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(trans));

			GLint matAmbientLoc = glGetUniformLocation(program, "material.ambient");
			GLint matDiffuseLoc = glGetUniformLocation(program, "material.diffuse");
			GLint matSpecularLoc = glGetUniformLocation(program, "material.specular");
			GLint matShineLoc = glGetUniformLocation(program, "material.shininess");

			glUniform3f(matAmbientLoc, myMaterial.ambient.r, myMaterial.ambient.g, myMaterial.ambient.b);
			//glUniform3f(matDiffuseLoc, myMaterial.diffuse.r, myMaterial.diffuse.g, myMaterial.diffuse.b);
			//glUniform3f(matSpecularLoc, myMaterial.specular.r, myMaterial.specular.g, myMaterial.specular.b);
			glUniform1f(matShineLoc, myMaterial.shininess);

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, myTextures[0]);
			glUniform1i(glGetUniformLocation(program, "material.diffuse"), 0);
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, myTextures[1]);
			glUniform1i(glGetUniformLocation(program, "material.specular"), 1);
			glUseProgram(program);
			glBindVertexArray(vao);
			glDrawArrays(GL_TRIANGLES, 0, 36);
			glBindVertexArray(0);
		}

		void Model::Update()
		{
			//this will be again overridden
		}

		void Model::SetProgram(GLuint aShaderName)
		{
			program = aShaderName;
		}

		GLuint Model::GetVao() const
		{
			return vao;
		}

		const std::vector<GLuint>& Model::GetVbos() const
		{
			return vbos;
		}

		void Model::Destroy()
		{
			glDeleteVertexArrays(1, &vao);
			glDeleteBuffers((GLsizei)vbos.size(), &vbos[0]);
			vbos.clear();
		}
	}
}