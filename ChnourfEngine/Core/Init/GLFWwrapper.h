#include <iostream>
#include "../../Dependencies/glew/glew.h"

struct GLFWwindow;

namespace Core
{
	class IListener;

	namespace Init
	{
		class GLFWwrapper
		{

		public:            
			static void Init(int w, int h, const char* aWindowName);

			static void Run();//called from outside
			static void Close();

			static void GetWindowWidthAndHeight(int& w, int& h);
			static GLFWwindow* GetWindow();

			static void PrintOpenGLInfo();

			static void SetListener(Core::IListener*& aIListener);

		private:
			static void ReshapeCallback(int aWidth, int aHeight);

			static Core::IListener* listener;
		};
	}
}