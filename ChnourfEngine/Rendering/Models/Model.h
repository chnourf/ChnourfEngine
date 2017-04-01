#pragma once
#include <vector>
#include "glm\gtx\rotate_vector.hpp"
#include "glm\gtc\type_ptr.hpp"
#include "../IGameObject.h"

namespace Rendering
{
	struct Material {
		Material() {}

		Material(const glm::vec3& anAmbient, const glm::vec3& aDiffuse, const glm::vec3& aSpecular, float aShininess) :
			ambient(anAmbient),
			diffuse(aDiffuse),
			specular(aSpecular),
			shininess(aShininess)
		{}

		glm::vec3 ambient;
		glm::vec3 diffuse;
		glm::vec3 specular;
		float shininess;
	};

	namespace Models
	{
		class Model :public IGameObject
		{
		public:
			Model();
			Model(const glm::mat4& aTransform);
			Model(const glm::mat4& aTransform, const Material& aMaterial);
			~Model();

			void Create();
			void Draw() override;
			void Update() override;
			void SetProgram(GLuint aShaderName) override;
			void Destroy() override;

			GLuint GetVao() const override;
			const std::vector<GLuint>& GetVbos() const override;
			const GLuint& GetProgram() override { return program; };

			glm::mat4 myTransform;

		protected:
			GLuint vao;
			GLuint program;
			std::vector<GLuint> vbos;

			std::vector<VertexFormat> myVertices;
			std::vector<GLuint> myTextures;
			Material myMaterial;

		private:
			void CreateTexture(GLuint& aTextureID, const std::string& aPath);
		};
	}
}