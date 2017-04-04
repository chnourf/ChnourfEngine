#ifndef  VertexFormat_H_
#define VertexFormat_H_

#include "glm\glm.hpp"

namespace Rendering
{
	struct Vertex
	{
		glm::vec3 position;
		glm::vec3 normal;
		glm::vec2 uv;

		Vertex()
		{
		}

		Vertex(const glm::vec3& aPos, const glm::vec3& aNormal, const glm::vec2& aUV)
		{
			position = aPos;
			normal = aNormal;
			uv = aUV;
		}
	};
}
#endif