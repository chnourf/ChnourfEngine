#pragma once
#include "Light.h"

class DirectionalLight : public Light
{
public:
	DirectionalLight() {};
	DirectionalLight(const glm::vec3& aDirection, const glm::vec3& anIntensity)
	{
		myDirection = glm::normalize(aDirection);
		myIntensity = anIntensity;
	}


	const glm::vec3& GetDirection() { return myDirection; }
	void SetDirection(const glm::vec3& aDirection) { myDirection = aDirection; }

private:
	glm::vec3 myDirection;
};

