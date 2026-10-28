#include "GameObject.h"
#include "GameFramework.h"
#include <iostream>

GAMEFRAMEWORK_API GameObject::GameObject(std::string name)
{
    this->m_name = name;
    m_transform = AddComponent<Transform>();
}

GAMEFRAMEWORK_API Transform* GameObject::GetTransform()
{
    return m_transform;
}

GAMEFRAMEWORK_API void GameObject::Destroy()
{
	for (auto keyValuePair : m_components)
	{
		for (auto* component : keyValuePair.second)
		{
			component->Destroy();
		}
	}

	m_components.clear();
}

GAMEFRAMEWORK_API void GameObject::AddComponentInternal(std::string type, GameComponent* component)
{
	auto search = m_components.find(type);

	if (search == m_components.end())
	{
		m_components[type] = {};
	}

	m_components[type].push_back(component);
	GameFramework::Instance->AddComponent(component);
}

GAMEFRAMEWORK_API void GameObject::LogComponentNotFoundError(std::string type)
{
    std::cerr << "GameObject: [" << m_name << "] has no component of type [" << type << "]";
}