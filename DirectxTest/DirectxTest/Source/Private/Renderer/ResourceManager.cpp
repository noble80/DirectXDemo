#include "stdafx.h"
#include "Renderer/ResourceManager.h"


ResourceManager::ResourceManager()
{
}


ResourceManager::~ResourceManager()
{
}

bool ResourceManager::RemoveResource(Resource* res, std::string name)
{
	ResourceID ID(&typeid(*res), name);
	auto it = m_Resources.find(ID);
	if(it == m_Resources.end())
		return false;
	
	m_Resources.erase(it);
	return true;
}

bool ResourceManager::Initialize()
{
	return true;
}

bool ResourceManager::Shutdown()
{
	// Create a map iterator and point to beginning of map
	auto it = m_Resources.begin();

	// Iterate over the map using Iterator till end.
	while(it != m_Resources.end())
	{
		it->second->Release();
		delete it->second;
		it++;
	}
	m_Resources.clear();

	return true;
}
