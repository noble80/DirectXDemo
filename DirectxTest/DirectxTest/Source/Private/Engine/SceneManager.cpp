#include "stdafx.h"
#include "Engine\SceneManager.h"

#include "Engine\CameraComponent.h"
#include "Engine\ModelComponent.h"
#include "Engine\TransformComponent.h"
#include "Engine\LightComponent.h"

SceneManager::SceneManager()
{
	m_ComponentManager.CreateComponentContainer<TransformComponent>(100);
	m_ComponentManager.CreateComponentContainer<ModelComponent>(100);
	m_ComponentManager.CreateComponentContainer<LightComponent>(4);
	m_ComponentManager.CreateComponentContainer<CameraComponent>(1);

	m_Entities.reserve(100);

}


SceneManager::~SceneManager()
{
}
