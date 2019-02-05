#include "stdafx.h"
#include "Renderer/GraphicsStructures.h"
#include <d3d11.h>

#include <tuple>

SimpleBuffer::SimpleBuffer()
{
	data = nullptr;
}

SimpleBuffer::~SimpleBuffer()
{
}

void SimpleBuffer::Release()
{
	if(data)
		data->Release();

}

GeometryBuffer::~GeometryBuffer()
{
}

void GeometryBuffer::Release()
{
	vertexBuffer.Release();
	indexBuffer.Release();
}

ConstantBuffer::ConstantBuffer()
{
	cpu = nullptr;
}

ConstantBuffer::~ConstantBuffer()
{
}

void ConstantBuffer::Release()
{
	if(cpu)
		free(cpu);
	gpu.Release();
}

Resource::~Resource()
{
}

const ResourceID & Resource::ReadID() const
{
	return id;
}
