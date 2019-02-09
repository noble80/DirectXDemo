#include "stdafx.h"
#include "Engine\SceneManager.h"

#include "Engine\CameraComponent.h"
#include "Engine\MeshComponent.h"
#include "Engine\TransformComponent.h"
#include "Engine\DirectionalLightComponent.h"
#include "Engine\PointLightComponent.h"
#include "Engine\SpotLightComponent.h"

SceneManager::SceneManager()
{
	m_ComponentManager.CreateComponentContainer<TransformComponent>(100);
	m_ComponentManager.CreateComponentContainer<MeshComponent>(100);
	m_ComponentManager.CreateComponentContainer<DirectionalLightComponent>(4);
	m_ComponentManager.CreateComponentContainer<PointLightComponent>(10);
	m_ComponentManager.CreateComponentContainer<SpotLightComponent>(10);
	m_ComponentManager.CreateComponentContainer<CameraComponent>(1);
}


SceneManager::~SceneManager()
{
	for(auto& it: m_Entities)
	{
		delete it.second;
	}
}

Entity * SceneManager::GetEntity(std::string name)
{
	EntityID id;
	id.name = name;

	auto it = m_Entities.find(id);
	if(it != m_Entities.end())
	{
		return it->second;
	}
	else
		return nullptr;
}

Entity * SceneManager::CreateEntity(std::string name)
{
	EntityID id;
	id.name = name;

	auto it = m_Entities.find(id);
	if(it == m_Entities.end())
	{
		Entity* e = new Entity(name);
		m_Entities.insert(std::make_pair(id, e));
		return e;
	}
	else
		return nullptr;
}
