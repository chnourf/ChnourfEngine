#include "UID.h"
#include <mutex>

static int UIDsCreated = 0;
static std::mutex mySafetyMutex;

UID::UID()
{
	while (!mySafetyMutex.try_lock())
	{ }

	myValue = UIDsCreated;
	UIDsCreated++;

	mySafetyMutex.unlock();
}