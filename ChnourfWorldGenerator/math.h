#pragma once
#include <algorithm>

float clamp(const float aValue, const float aMin, const float aMax)
{
	return std::fmin(std::fmax(aValue, aMin), aMax);
}