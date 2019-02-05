#pragma once

#include "stdafx.h"

#include <d3d11.h>

struct ResourceID
{
	const std::type_info* type;
	std::string name;

	inline bool operator<(const ResourceID& rhs) const
	{
		return std::tie(type, name) < std::tie(rhs.type, rhs.name);
	};

	ResourceID() {};
	ResourceID(const std::type_info* _type, std::string _name) : type(_type), name(_name) {};
};

struct Resource
{
	friend class ResourceManager;

	//ResourceID ID;
	virtual void Release() = 0;

	virtual ~Resource() = 0;

	const ResourceID& ReadID() const;
private:
	ResourceID id;
};

struct SimpleBuffer : Resource
{
	ID3D11Buffer* data;
	uint32_t size;

	SimpleBuffer();
	virtual ~SimpleBuffer() override;

	virtual void Release() override;
};

struct GeometryBuffer : Resource
{
	SimpleBuffer vertexBuffer;
	SimpleBuffer indexBuffer;

	virtual ~GeometryBuffer() override;

	virtual void Release() override;
};

struct ID3D11VertexShader;
struct ID3D11PixelShader;


struct ConstantBuffer : Resource
{
	void* cpu;
	SimpleBuffer gpu;

	ConstantBuffer();
	virtual ~ConstantBuffer() override;

	virtual void Release() override;
};

