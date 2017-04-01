#pragma once
#include "Light.h"

class PointLight : public Light
{
public:
	PointLight() {};
	PointLight(const glm::vec3& aPosition, const glm::vec3& anIntensity) :
		myPosition(aPosition)
	{
		myIntensity = anIntensity;
	}
	~PointLight() {};

	const glm::vec3& GetPosition() { return myPosition; }

private:
	glm::vec3 myPosition;
};

