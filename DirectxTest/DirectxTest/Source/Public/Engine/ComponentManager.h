#pragma once
#include "stdafx.h"

#include "Component.h"

struct ComponentContainerBase
{
public:
	virtual ~ComponentContainerBase() {};
};

template<class T>
struct ComponentContainer : public ComponentContainerBase
{
	typedef std::vector<T> vector_type;

	virtual ~ComponentContainer() override {};

	vector_type vector;
};

class ComponentManager
{
public:
	ComponentManager();
	~ComponentManager();

	template<class T>
	std::vector<T>* CreateComponentContainer(int max)
	{
		ComponentContainer<T>* container = new ComponentContainer<T>;
		auto it = m_ComponentVectors.insert(std::make_pair(&typeid(T), container));
		if(it.second == false)
		{
			container->vector.clear();
			delete container;
			return nullptr;
		}
		container->vector.reserve(max);
		return &container->vector;
	}

	template<class T>
	T* CreateComponent()
	{
		T comp;
		ComponentContainer<T>* container;
		std::vector<T>* vec;

		auto it = m_ComponentVectors.find(&typeid(T));
		if(it != m_ComponentVectors.end())
		{
			container = static_cast<ComponentContainer<T>*>(it->second);
			vec = &container->vector;

			if(vec->capacity() == vec->size())
				return nullptr;

			vec->push_back(comp);
			return &vec->back();
		}
		else
			return nullptr;
	}

	template<class T>
	std::vector<T>* GetComponents()
	{
		ComponentContainer<T>* container;
		std::vector<T>* vec;

		auto it = m_ComponentVectors.find(&typeid(T));
		if(it != m_ComponentVectors.end())
		{
			container = static_cast<ComponentContainer<T>*>(it->second);
			vec = &container->vector;
			return vec;
		}
		else
			return nullptr;
	}

private:
	std::map<const std::type_info*, ComponentContainerBase*, type_infoComparator> m_ComponentVectors;
};

