#include "Core\Init\InitGLUT.h"
#include "Managers\SceneManager.h"
#include "WorldGenerator\TerrainManager.h"

using namespace Core;
using namespace Init;

int main(int argc, char **argv)
{
	WindowInfo window(std::string("Chnourf Engine"), 400, 200, 1600, 900, true);

	ContextInfo context(4, 5, true);
	FramebufferInfo frameBufferInfo(true, true, true, true);
	Init_GLUT::Init(window, context, frameBufferInfo);

	Manager::TerrainManager::Create();
	
	Manager::SceneManager::Create();
	Manager::SceneManager::GetInstance()->Initialize(window);
	IListener* scene = Manager::SceneManager::GetInstance();
	Init::Init_GLUT::SetListener(scene);

	Init_GLUT::Run();

	Manager::SceneManager::Destroy();

	return 0;
}