#include "Core\Init\GLFWwrapper.h"
#include "Managers\SceneManager.h"
#include "Managers\InputManager.h"
#include "WorldGenerator\TerrainManager.h"
#include "Core\Time.h"
#include "Dependencies\GLFW\glfw3.h"

using namespace Core;
using namespace Init;

int main(int argc, char **argv)
{
	GLFWwrapper::Init(1600, 900, "Chnourf Engine");

	Time::Create();
	Manager::InputManager::Create();

	Manager::TerrainManager::Create();
	
	Manager::SceneManager::Create();
	Manager::SceneManager::GetInstance()->Initialize();
	IListener* scene = Manager::SceneManager::GetInstance();
	GLFWwrapper::SetListener(scene);

	GLFWwrapper::Run();

	Manager::SceneManager::Destroy();

	Manager::TerrainManager::Destroy();

	Manager::InputManager::Destroy();

	glfwTerminate();

	return 0;
}