#ifndef  VertexFormat_H_
#define VertexFormat_H_

#include "glm\glm.hpp"

namespace Rendering
{
	struct VertexFormat
	{
		glm::vec3 position;
		glm::vec3 normal;
		glm::vec2 uv;
		//glm::vec4 color;

		VertexFormat(const glm::vec3& aPos, const glm::vec3& aNormal, const glm::vec2& aUV)
		{
			position = aPos;
			normal = aNormal;
			uv = aUV;
			//color = aColor;
		}
	};
}
#endif