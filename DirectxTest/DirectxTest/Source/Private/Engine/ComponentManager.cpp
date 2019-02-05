#include "stdafx.h"
#include "Engine/ComponentManager.h"


ComponentManager::ComponentManager()
{
}


ComponentManager::~ComponentManager()
{
	for(auto& pair : m_ComponentVectors)
	{
		delete pair.second;
	}
}
