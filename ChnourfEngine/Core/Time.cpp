#include "Time.h"
#include <ctime>

std::clock_t startTime = std::clock();

Time::Time() :
	myCurrentTime(0.0),
	myPreviousTime(0.0)
{
}

void Time::Update()
{
	auto elapsedTime = (std::clock() - startTime) / (double)(CLOCKS_PER_SEC);

	myPreviousTime = myCurrentTime;
	myCurrentTime = elapsedTime;
}