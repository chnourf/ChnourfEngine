#pragma once
#include "Light.h"

class DirectionalLight : public Light
{
public:
	DirectionalLight() {};
	DirectionalLight(const glm::vec3& aDirection, const glm::vec3& anIntensity) :
		myDirection(aDirection)
	{
		myIntensity = anIntensity;
	}


	const glm::vec3& GetDirection() { return myDirection; }

private:
	glm::vec3 myDirection;
};

