#pragma once

class Entity;

class Component
{
public:
	Component();
	virtual void Initialize(Entity* owner);
	virtual ~Component();

	inline Entity* GetOwner() const { return m_Owner; }

protected:
	Entity* m_Owner;
};

