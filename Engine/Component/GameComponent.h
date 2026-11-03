#pragma once
#include "GameFrameworkExports.h"
#include "Utils/SimpleMath.h"
#include "Material.h"

class GameObject;
class GameFramework;

class GameComponent
{
public:
	GAMEFRAMEWORK_API GameComponent(GameObject* gameObject);
	GAMEFRAMEWORK_API virtual void Update(float deltaTime) = 0;
	GAMEFRAMEWORK_API virtual void Destroy() = 0;

	
	GameObject* m_gameObject;
	bool enabled;
};

