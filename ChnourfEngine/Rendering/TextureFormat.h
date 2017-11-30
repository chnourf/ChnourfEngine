#ifndef  TextureFormat_H_
#define TextureFormat_H_

#include "../Dependencies/GLFW/glfw3.h"

#include "../Dependencies/Assimp/types.h"

namespace Rendering
{
	enum TextureType
	{
		diffuse,
		specular,
		normal,
	};

	struct TextureFormat
	{
		GLuint myId;
		TextureType myType;
		aiString myPath;  // We store the path of the texture to compare with other textures
	};
}
#endif