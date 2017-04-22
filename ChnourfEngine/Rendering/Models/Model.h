#pragma once
#include <vector>
#include "glm\gtx\rotate_vector.hpp"
#include "glm\gtc\type_ptr.hpp"
#include "../IGameObject.h"
#include "Mesh.h"

struct aiMaterial;
struct aiMesh;
struct aiNode;
struct aiScene;
enum aiTextureType;

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
		glm::vec3 diffuse; //not used
		glm::vec3 specular; //not used
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
			Model(const GLchar* aPath, const glm::mat4& aTransform);
			~Model();

			void Create();
			void Draw(const Manager::ShaderManager* aShaderManager) override;
			void DrawForShadowMap(const GLuint aShadowMapProgram) override;
			void Update() override;
			void SetProgram(GLuint aShaderName) override;
			void Destroy() override;

			glm::mat4 myTransform;

		protected:

		private:
			//to be moved in mesh ? Dunno
			GLuint myProgram;
			
			std::string myDirectory;
			std::vector<Mesh*> myMeshes;
			void ProcessNode(aiNode* node, const aiScene* scene);
			Mesh* ProcessMesh(aiMesh* mesh, const aiScene* scene);
			Material myMaterial;
			void CreateTexture(GLuint& aTextureID, const std::string& aPath);
			void LoadModel(const std::string& aPath);

			std::vector<TextureFormat> LoadMaterialTextures(aiMaterial* aMat, aiTextureType anAiType, TextureType aType);
		};
	}
}