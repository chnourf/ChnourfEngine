#pragma once

#include "glm\glm.hpp"

class Light
{
public:
	Light() {};
	~Light() {};

	const glm::vec3& GetIntensity() { return myIntensity; }

protected:
	glm::vec3 myIntensity;
};

