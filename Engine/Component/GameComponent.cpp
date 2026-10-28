#include "GameComponent.h"

GAMEFRAMEWORK_API GameComponent::GameComponent(GameObject* gameObject)
{
	m_gameObject = gameObject;
	enabled = true;
}