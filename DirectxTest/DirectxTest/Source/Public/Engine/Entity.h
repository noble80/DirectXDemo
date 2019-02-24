#pragma once
#include "stdafx.h"

#include "Component.h"

struct EntityID
{
	std::string name;

	inline bool operator<(const EntityID& rhs) const
	{
		return std::tie(name) < std::tie(rhs.name);
	};
};

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


	std::string GetName() const { return m_Name; };

	inline void AddComponent(Component* comp)
	{
		m_Components.insert(std::make_pair(&typeid(*comp), comp));
	}
private:
	std::multimap<const std::type_info*, Component*> m_Components;
	std::string m_Name;
};

