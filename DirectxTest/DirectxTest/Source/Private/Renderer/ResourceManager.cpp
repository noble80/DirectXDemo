#include "stdafx.h"
#include "Renderer/ResourceManager.h"

#include <d3d11.h>

ResourceManager::ResourceManager()
{
}


ResourceManager::~ResourceManager()
{
}

bool ResourceManager::RemoveResource(Resource* res)
{
	ResourceID ID(&typeid(*res), res->id.name);
	auto it = m_Resources.find(ID);
	if(it == m_Resources.end())
		return false;
	
	it->second->Release();
	delete it->second;
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
