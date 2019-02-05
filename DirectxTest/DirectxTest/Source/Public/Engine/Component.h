#pragma once

class Entity;

class Component
{
public:
	Component();
	virtual void Initialize(Entity* owner);
	virtual ~Component();

protected:
	Entity* m_Owner;
};

