#pragma once

#include "glm\glm.hpp"

class Light
{
public:
	Light() {};
	~Light() {};

	const glm::vec3& GetIntensity() { return myIntensity; }
	void SetIntensity(const glm::vec3& anIntensity) { myIntensity = anIntensity; }

protected:
	glm::vec3 myIntensity;
};

