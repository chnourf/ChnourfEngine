
#include "ContextInfo.h"
#include "FrameBufferInfo.h"
#include "WindowInfo.h"
#include <iostream>
#include "InitGLEW.h"

namespace Core
{
	class IListener;

	namespace Init
	{
		class Init_GLUT
		{

		public:            
			static void Init(const Core::WindowInfo& aWindow, const Core::ContextInfo& aContext, const Core::FramebufferInfo& aFramebufferInfo);

			static void Run();//called from outside
			static void Close();

			void EnterFullscreen();
			void ExitFullscreen();

			static void PrintOpenGLInfo(const Core::WindowInfo& aWindowInfo,	const Core::ContextInfo& aContext);

			static void SetListener(Core::IListener*& aIListener);

		private:
			static void IdleCallback();
			static void DisplayCallback();
			static void ReshapeCallback(int aWidth, int aHeight);
			static void CloseCallback();

			static Core::IListener* listener;
			static Core::WindowInfo windowInformation;
		};
	}
}