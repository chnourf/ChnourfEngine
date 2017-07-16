#include "Singleton.h"

class Time : public Singleton<Time>
{
public :
	Time();

	void Update();
	__forceinline double GetTime() { return myCurrentTime; }
	__forceinline double GetElapsedTimeSinceLastFrame() { return myCurrentTime - myPreviousTime; }
private :
	double myCurrentTime;
	double myPreviousTime;
};