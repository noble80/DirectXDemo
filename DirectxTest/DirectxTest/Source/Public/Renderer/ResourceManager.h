#pragma once
#include "stdafx.h"
#include "GraphicsStructures.h"

struct ID3D11Buffer;
struct D3D11_SUBRESOURCE_DATA;
struct D3D11_BUFFER_DESC;

class ResourceManager
{
public:
	ResourceManager();
	~ResourceManager();

	template<typename T>
	T* GetResource(std::string name)
	{
		ResourceID ID(&typeid(T), name);
		auto it = m_Resources.find(ID);
		if(it == m_Resources.end())
			return nullptr;
		else
			return static_cast<T*>(it->second);
	}

	template<typename T>
	T* CreateResource(std::string name)
	{
		ResourceID ID(&typeid(T), name);
		T* res = new T;
		auto it = m_Resources.find(ID);
		if(it == m_Resources.end())
		{
			m_Resources.insert(std::make_pair(ID, res));
			return res;
		}
		else
			return nullptr;
	}

	bool RemoveResource(Resource* res, std::string name);

	ID3D11Buffer* CreateD3DBuffer(D3D11_BUFFER_DESC* desc, D3D11_SUBRESOURCE_DATA* InitData);

	bool Initialize();
	bool Shutdown();
private:

	std::map<ResourceID, Resource*> m_Resources;
};

