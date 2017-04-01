namespace Core
{
	//OpenGL versions
	struct ContextInfo
	{
		int majorVersion;
		int minorVersion;
		bool core;

		ContextInfo():
			majorVersion(3),
			minorVersion(3),
			core(true)
		{}

		ContextInfo(int aMajorVersion, int aMinorVersion, bool aCore):
			majorVersion(aMajorVersion),
			minorVersion(aMinorVersion),
			core(aCore)
		{}

		ContextInfo(const ContextInfo& aContextInfo):
			majorVersion(aContextInfo.majorVersion),
			minorVersion(aContextInfo.minorVersion),
			core(aContextInfo.core)
		{}

		void operator=(const ContextInfo& aContextInfo)
		{
			majorVersion = aContextInfo.majorVersion;
			minorVersion = aContextInfo.minorVersion;
			core = aContextInfo.core;
		}
	};
}