#pragma once
#include "stdafx.h"

#include "Entity.h"
#include "ComponentManager.h"

class SceneManager
{
public:
	SceneManager();
	~SceneManager();

	template<class T>
	T* CreateComponent(Entity* owner);

	template<class T>
	std::vector<T>* GetComponents();

	inline std::vector<Entity>& GetEntities() { return m_Entities; };

	inline Entity* CreateEntity(std::string name)
	{
		m_Entities.push_back(Entity(name));
		return &m_Entities.back();
	}

private:
	ComponentManager m_ComponentManager;
	std::vector<Entity> m_Entities;
	
};

template<class T>
T* SceneManager::CreateComponent(Entity* owner)
{
	T* component;
	component = m_ComponentManager.CreateComponent<T>();
	component->Initialize(owner);
	owner->AddComponent(component);
	return component;
}
template<class T>
std::vector<T>* SceneManager::GetComponents()
{
	return m_ComponentManager.GetComponents<T>();
}