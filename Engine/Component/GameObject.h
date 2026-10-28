#pragma once
#include "GameFrameworkExports.h"
#include "Component/Transform/Transform.h"
#include <map>
#include <string>
#include <vector>
#include <typeinfo>

class GameComponent;
class GameFramework;

class GameObject
{
public:
	GAMEFRAMEWORK_API GameObject(std::string name);
	GAMEFRAMEWORK_API Transform* GetTransform();

	template <typename T>
	T* GetComponent();

	template <typename T>
	std::vector<T*> GetAllComponents();

	template <typename T>
	T* AddComponent();

	GAMEFRAMEWORK_API virtual void Destroy();


private:
	GAMEFRAMEWORK_API void AddComponentInternal(std::string type, GameComponent* gameComponent);
	GAMEFRAMEWORK_API void LogComponentNotFoundError(std::string type);

	Transform* m_transform;
	std::map<std::string, std::vector<GameComponent*>> m_components;
	std::string m_name;
};

template<typename T>
inline T* GameObject::GetComponent()
{
	auto type = std::string(typeid(T).name());
	
	auto search = m_components.find(type);
	if (search == m_components.end())
	{
		LogComponentNotFoundError(type);
		return nullptr;
	}

	auto list = search->second;
	if (list.size() <= 0)
	{
		LogComponentNotFoundError(type);
		return nullptr;
	}

	return list[0];
}

template<typename T>
inline std::vector<T*> GameObject::GetAllComponents()
{
	auto type = std::string(typeid(T).name());

	auto search = m_components.find(type);
	if (search == m_components.end())
	{
		LogComponentNotFoundError(type);
		return {};
	}

	return search->second;
}

template<typename T>
inline T* GameObject::AddComponent()
{
	auto type = std::string(typeid(T).name());

	auto* component = new T(this);
	auto* gameComponent = dynamic_cast<GameComponent*>(component);

	if (gameComponent == nullptr)
		return nullptr;

	AddComponentInternal(type, gameComponent);

	return component;
}
