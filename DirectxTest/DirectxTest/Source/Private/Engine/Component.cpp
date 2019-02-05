#include "stdafx.h"
#include "Engine/Component.h"

Component::Component()
{
}

void Component::Initialize(Entity * owner)
{
	m_Owner = owner;
}


Component::~Component()
{
}
