#pragma once
#include <chrono>
#include <vector>

#include "Camera/CameraController.h"
#include "Component/Render/RenderComponent.h"
#include "Component/Render/Light/DirectionalLight.h"
#include "Component/Render/Light/PointLight.h"
#include "InputDevice.h"
#include "Model.h"
#include "GBuffer.h"
#include "ShaderFlags.h"
#include <map>

class PhysicalBoxComponent;
class PhysicalSphereComponent;
class ParticleSystemSorted;
class DisplayWin;
class GLFWwindow;

namespace RenderAPI
{
	class RenderAPI;
}

class GameFramework
{
public:
	GAMEFRAMEWORK_API GameFramework(const std::string& applicationName);
	GAMEFRAMEWORK_API virtual void Init(int windowWidth, int windowHeight);
	GAMEFRAMEWORK_API virtual void Run();
	GAMEFRAMEWORK_API virtual void UpdateFrameCount(unsigned int& frameCount, float& totalTimeClamped);
	GAMEFRAMEWORK_API virtual void Update();

	GAMEFRAMEWORK_API virtual GameObject* CreateGameObject(std::string name);
	GAMEFRAMEWORK_API virtual void AddParticleSystem(ParticleSystemSorted* particleSystem);

	GAMEFRAMEWORK_API PhysicalComponent* Intersects(PhysicalComponent* queryingObject);

	//GAMEFRAMEWORK_API virtual GameComponent* RayIntersectsSomething(PhysicalBoxComponent* queryingBox, DirectX::SimpleMath::Vector3 origin, DirectX::SimpleMath::Vector3 currentSpeed);
	
	GAMEFRAMEWORK_API virtual void FreeGameResources();
	GAMEFRAMEWORK_API virtual void SetCameraController(int cameraIdx);

	GAMEFRAMEWORK_API static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
	GAMEFRAMEWORK_API static void CursorPosCallback(GLFWwindow* window, double xpos, double ypos);
	GAMEFRAMEWORK_API static void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
	GAMEFRAMEWORK_API static void ScrollCallback(GLFWwindow* window, double xoffset, double yoffset);


	GAMEFRAMEWORK_API static GameFramework* Instance;
	GAMEFRAMEWORK_API static DisplayWin* displayWin;
	GAMEFRAMEWORK_API static InputDevice* inputDevice;

	RenderAPI::RenderAPI* m_renderer;

	int screenWidth;
	int screenHeight;
	std::string applicationName;

	std::vector<GameObject*> gameObjects;
	std::vector<GameComponent*> gameComponents;
	std::vector<PhysicalBoxComponent*> physicalBoxComponents;
	std::vector<PhysicalSphereComponent*> physicalSphereComponents;

	Camera* camera;
	std::vector<CameraController*> cameraControllers;

	std::chrono::time_point<std::chrono::steady_clock> PrevTime;
	float deltaTime;

private:
	GAMEFRAMEWORK_API virtual void AddComponent(GameComponent* gameComponent);
	GAMEFRAMEWORK_API void AddComponentInternal(GameComponent* gameComponent);

	friend class GameObject;
};

