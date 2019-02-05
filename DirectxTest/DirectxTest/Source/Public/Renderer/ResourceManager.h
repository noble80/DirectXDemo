#pragma once
#include "stdafx.h"
#include "GraphicsStructures.h"
class ResourceManager
{
public:
	ResourceManager();
	~ResourceManager();

	template<typename T>
	T* GetResource(std::string name)
	{
		ResourceID ID(&typeid(T), name);
		auto it = m_Resources.find(ID);
		if(it == m_Resources.end())
			return nullptr;
		else
			return static_cast<T*>(it->second);
	}

	template<typename T>
	T* CreateResource(std::string name)
	{
		ResourceID ID(&typeid(T), name);
		T* res = new T;
		auto it = m_Resources.find(ID);
		if(it == m_Resources.end())
		{
			m_Resources.insert(std::make_pair(ID, res));
			return res;
		}
		else
			return nullptr;
	}

	bool RemoveResource(Resource* res, std::string name);

	bool Initialize();
	bool Shutdown();
private:

	std::map<ResourceID, Resource*> m_Resources;
};

