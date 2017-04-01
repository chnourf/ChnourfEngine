namespace Core
{
	class IListener
	{
	public:
		virtual ~IListener() = 0;

		//drawing functions
		virtual void NotifyBeginFrame() = 0;
		virtual void NotifyDisplayFrame() = 0;
		virtual void NotifyEndFrame() = 0;
		virtual void NotifyReshape(int aWidth, int aHeight, int aPreviousWidth, int aPreviousHeight) = 0;
	};

	inline IListener::~IListener() {} //pure virtual destructor
}