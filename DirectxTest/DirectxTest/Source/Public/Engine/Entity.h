#pragma once
#include "stdafx.h"

#include "Component.h"

class Entity
{
public:
	Entity(std::string name);
	~Entity();

	template<class T>
	T* GetComponent()
	{
		auto it = m_Components.find(&typeid(T));

		if(it == m_Components.end()) return nullptr;

		return static_cast<T*>(it->second);
	}


	inline void AddComponent(Component* comp)
	{
		m_Components.insert(std::make_pair(&typeid(*comp), comp));
	}
private:
	std::multimap<const std::type_info*, Component*> m_Components;
	std::string m_Name;
};

