#include "stdafx.h"
#include "Renderer\Renderer.h"

#include "Renderer/Window.h"
#include "Renderer/ShaderUtilities.h"
#include "Renderer/ShaderBuffers.h"
#include "Renderer/ResourceManager.h"

#include "Renderer\RenderTexture2D.h"
#include "Renderer\Texture2D.h"
#include "Renderer\Material.h"
#include "Renderer\Mesh.h"

#include "Engine/ModelComponent.h"
#include "Engine/DirectionalLightComponent.h"
#include "Engine/SpotLightComponent.h"
#include "Engine/PointLightComponent.h"
#include "Engine/CameraComponent.h"
#include "Importer/DDSTextureLoader.h"

#include "StringUtility.h"

#include "Math/Vertex.h"

#include <d3d11_1.h>
#include <DirectXPackedVector.h>

// include the Direct3D Library file
#pragma comment (lib, "d3d11.lib")

using namespace DirectX;
using namespace Microsoft::WRL;

Renderer::Renderer()
{
	m_Swapchain = nullptr;
	m_Device = nullptr;
	m_Context = nullptr;
	m_RenderTargetView = nullptr;
}


Renderer::~Renderer()
{
}

bool Renderer::Initialize(Window * window)
{
	HRESULT hr = S_OK;

	Vector2 dimensions = window->GetDimensions();

	m_ResourceManager = new ResourceManager;
	m_ResourceManager->Initialize();

#pragma region DEVICE_CREATION
	{
		ComPtr<ID3D11Device> device = nullptr;
		ComPtr<ID3D11DeviceContext> context = nullptr;

	#ifdef _DEBUG
		UINT flags = D3D11_CREATE_DEVICE_DEBUG;
	#else
		UINT flags = 0;
	#endif
		D3D_FEATURE_LEVEL	FeatureLevels[] =
		{
			D3D_FEATURE_LEVEL_11_1,
			D3D_FEATURE_LEVEL_11_0,
			D3D_FEATURE_LEVEL_10_1,
			D3D_FEATURE_LEVEL_10_0,
		};
		UINT				numFeatureLevels = ARRAYSIZE(FeatureLevels);
		D3D_FEATURE_LEVEL	FeatureLevelObtained;


		hr = D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, flags, FeatureLevels, numFeatureLevels,
			D3D11_SDK_VERSION, &device, &FeatureLevelObtained, &context);

		if(hr == E_INVALIDARG)
		{
			// DirectX 11.0 platforms will not recognize D3D_FEATURE_LEVEL_11_1 so we need to retry without it
			hr = D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, flags, &FeatureLevels[1], numFeatureLevels,
				D3D11_SDK_VERSION, &device, &FeatureLevelObtained, &context);
		}

		if(SUCCEEDED(hr))
		{
			hr = device.As(&m_Device);
			if(SUCCEEDED(hr))
			{
				hr = context.As(&m_Context);
			}
		}
		if(FAILED(hr))
		{
			return false;
		}
	}
#pragma endregion DEVICE_CREATION

#pragma region SWAPCHAIN_CREATION
	// Obtain DXGI factory from device (since we used nullptr for pAdapter above)
	{
		ComPtr<IDXGIFactory1> dxgiFactory = nullptr;
		{
			ComPtr<IDXGIDevice> dxgiDevice = nullptr;
			hr = m_Device.As(&dxgiDevice);
			if(SUCCEEDED(hr))
			{
				ComPtr<IDXGIAdapter> adapter = nullptr;
				hr = dxgiDevice->GetAdapter(&adapter);
				if(SUCCEEDED(hr))
				{
					hr = adapter->GetParent(__uuidof(IDXGIFactory1), &dxgiFactory);
				}
			}
		}
		// Create swap chain
		ComPtr<IDXGIFactory2> dxgiFactory2 = nullptr;
		hr = dxgiFactory.As(&dxgiFactory2);
		if(dxgiFactory2)
		{
			DXGI_SWAP_CHAIN_DESC1 sd{};
			sd.Width = static_cast<UINT>(dimensions.x);
			sd.Height = static_cast<UINT>(dimensions.y);
			sd.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
			sd.Stereo = false;
			sd.SampleDesc.Count = 1; // Don't use multi-sampling.
			sd.SampleDesc.Quality = 0;
			sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
			sd.BufferCount = 2;
			sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
			sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH; //| DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;

			DXGI_SWAP_CHAIN_FULLSCREEN_DESC fd{};
			fd.RefreshRate.Numerator = 60;
			fd.RefreshRate.Denominator = 1;
			fd.Windowed = !window->IsFullscreen();
			fd.Scaling = DXGI_MODE_SCALING_STRETCHED;

			hr = dxgiFactory2->CreateSwapChainForHwnd(m_Device.Get(), window->GetHandle(), &sd, &fd, nullptr, &m_Swapchain);
		}
		dxgiFactory->MakeWindowAssociation(window->GetHandle(), DXGI_MWA_NO_ALT_ENTER);

		if(FAILED(hr))
			return false;
	}
#pragma endregion SWAPCHAIN_CREATION

#pragma region BACKBUFFER_CREATION
	/* Backbuffer */
	{
		// get the address of the back buffer
		ComPtr<ID3D11Texture2D> backBuffer;
		hr = m_Swapchain->GetBuffer(
			0, // we are using one backbuffer so we only need the first one
			__uuidof(ID3D11Texture2D), // get ID of texture type
			&backBuffer);

		if(FAILED(hr))
			return false;

		// use the back buffer address to create the render target
		hr = m_Device->CreateRenderTargetView(backBuffer.Get(), NULL, &m_RenderTargetView);

		if(FAILED(hr))
			return false;
	}
#pragma endregion BACKBUFFER_CREATION

#pragma region STENCIL_CREATION
	/* Stencil */
	{
		// Create depth stencil texture
		ComPtr<ID3D11Texture2D> stencilTx;
		D3D11_TEXTURE2D_DESC descDepth{};
		descDepth.Width = static_cast<UINT>(dimensions.x);
		descDepth.Height = static_cast<UINT>(dimensions.y);
		descDepth.MipLevels = 1;
		descDepth.ArraySize = 1;
		descDepth.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		descDepth.SampleDesc.Count = 1;
		descDepth.SampleDesc.Quality = 0;
		descDepth.Usage = D3D11_USAGE_DEFAULT;
		descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;
		descDepth.CPUAccessFlags = 0;
		descDepth.MiscFlags = 0;
		hr = m_Device->CreateTexture2D(&descDepth, nullptr, &stencilTx);
		if(FAILED(hr))
			return hr;

		// Create the depth stencil view
		D3D11_DEPTH_STENCIL_VIEW_DESC descDSV{};
		descDSV.Format = descDepth.Format;
		descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		descDSV.Texture2D.MipSlice = 0;
		hr = m_Device->CreateDepthStencilView(stencilTx.Get(), &descDSV, &m_DepthStencilView);
		if(FAILED(hr))
			return hr;
	}
#pragma endregion STENCIL_CREATION

#pragma region VIEWPORT_CREATION
	/* Viewport */

	// Set the viewport
	m_ActiveCameraViewport = new D3D11_VIEWPORT{};

	m_ActiveCameraViewport->TopLeftX = 0;
	m_ActiveCameraViewport->TopLeftY = 0;

	m_ActiveCameraViewport->Width = dimensions.x;
	m_ActiveCameraViewport->Height = dimensions.y;

	m_ActiveCameraViewport->MaxDepth = 1.0f;
	m_ActiveCameraViewport->MinDepth = 0.0f;
#pragma endregion VIEWPORT_CREATION

	// Create the sample state wrap
	{
		D3D11_SAMPLER_DESC sampDesc = {};
		sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
		sampDesc.MipLODBias = 0.0f;
		sampDesc.MaxAnisotropy = 1;
		sampDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
		sampDesc.BorderColor[0] = 0;
		sampDesc.BorderColor[1] = 0;
		sampDesc.BorderColor[2] = 0;
		sampDesc.BorderColor[3] = 0;
		sampDesc.MinLOD = 0;
		sampDesc.MaxLOD = D3D11_FLOAT32_MAX;

		hr = m_Device->CreateSamplerState(&sampDesc, &m_SamplerLinearWrap);
		if(FAILED(hr))
			return hr;

	// Create the sample state clamp
		float black[] = {0.f, 0.f, 0.f, 1.f};
		CD3D11_SAMPLER_DESC SamDescShad(D3D11_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT,
			D3D11_TEXTURE_ADDRESS_BORDER,
			D3D11_TEXTURE_ADDRESS_BORDER,
			D3D11_TEXTURE_ADDRESS_BORDER,
			0.f,
			0,
			D3D11_COMPARISON_LESS,
			black,
			0,
			0);
		hr = m_Device->CreateSamplerState(&SamDescShad, &m_ShadowSampler);
		if(FAILED(hr))
			return hr;
	}

	CreateRasterizerStates();
	InitializeConstantBuffers();
	InitializeDefaultShaders();

	return true;
}

bool Renderer::Update()
{

	return true;
}

bool Renderer::Shutdown()
{
	HRESULT hr;

	if(m_Swapchain)
	{
		hr = m_Swapchain->SetFullscreenState(FALSE, NULL);
	}

	if(m_ResourceManager)
	{
		m_ResourceManager->Shutdown();
		delete m_ResourceManager;
	}

	if(m_ActiveCameraViewport)
		delete m_ActiveCameraViewport;

	if(m_DirectionalLightViewport)
		delete m_DirectionalLightViewport;

	return true;
}

bool Renderer::InitializeGeometryPass()
{
	// Clear the back buffer
	float color[4] = {0.0f, 0.2f, 0.4f, 1.0f};
	m_Context->ClearRenderTargetView(m_RenderTargetView.Get(), color);

	// Clear the depth buffer to 1.0 (max depth)
	m_Context->ClearDepthStencilView(m_DepthStencilView.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);
	m_Context->OMSetRenderTargets(1, m_RenderTargetView.GetAddressOf(), m_DepthStencilView.Get());
	m_Context->RSSetViewports(1, m_ActiveCameraViewport);

	m_View = m_ActiveCamera->GetViewMatrix();
	m_ViewProjection = m_View * m_ActiveCamera->GetOrtographicProjectionMatrix();

	m_Context->RSSetState(m_SceneRasterizerState.Get());

	return true;
}

bool Renderer::Draw()
{
	return false;
}



bool Renderer::RenderFrame(void)
{
	RenderShadowMaps();
	InitializeGeometryPass();
	for(auto& model : *m_ActiveModels)
	{
		for(auto& mesh : model.GetMeshes())
		{
			DrawMesh(mesh, model.GetTransformMatrix());
		}
	}
	ID3D11ShaderResourceView* null = nullptr;
	m_Context->PSSetShaderResources(2, 1, &null);

	HRESULT hr;
	// switch the back buffer and the front buffer
	DXGI_PRESENT_PARAMETERS params{};
	UINT flags = 0; // | DXGI_PRESENT_ALLOW_TEARING;
	hr = m_Swapchain->Present1(1, flags, &params);

	if(FAILED(hr))
	{
		return false;
	}

	return true;
}

GeometryBuffer* Renderer::CreateGeometryBuffer(std::string name, std::vector<Vertex> *vertices, std::vector<uint32_t> indices)
{
	HRESULT hr;

	GeometryBuffer* buffer = GetResourceManager()->CreateResource<GeometryBuffer>(name);

	if(buffer)
	{
		buffer->indexBuffer.size = static_cast<uint32_t>(indices.size());
		buffer->vertexBuffer.size = static_cast<uint32_t>(vertices->size());

		D3D11_BUFFER_DESC bd{};
		bd.Usage = D3D11_USAGE_DEFAULT;
		bd.ByteWidth = sizeof(Vertex) * buffer->vertexBuffer.size;
		bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bd.CPUAccessFlags = 0;

		D3D11_SUBRESOURCE_DATA InitData{};
		InitData.pSysMem = vertices->data();
		hr = m_Device->CreateBuffer(&bd, &InitData, &buffer->vertexBuffer.data);
		if(SUCCEEDED(hr))
		{
			bd.Usage = D3D11_USAGE_DEFAULT;
			bd.ByteWidth = sizeof(uint32_t) * buffer->indexBuffer.size;
			bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
			bd.CPUAccessFlags = 0;
			InitData.pSysMem = indices.data();
			hr = m_Device->CreateBuffer(&bd, &InitData, &buffer->indexBuffer.data);

			if(SUCCEEDED(hr))
				return buffer;
		}

		buffer->Release();
		GetResourceManager()->RemoveResource(buffer, name);
	}

	return nullptr;
}

Material* Renderer::CreateMaterialFromFile(std::string name)
{
	Material* mat = GetResourceManager()->CreateResource<Material>(name);
	std::vector<std::byte> VSBytes;
	std::vector<std::byte> PSBytes;

	bool foundVS = false;
	bool foundPS = false;

	std::string fullPath = "../Shaders/" + name + "_PS.cso";
	if(ShaderUtilities::LoadShaderFromFile(fullPath, PSBytes))
	{
		foundPS = true;
	}
	else
		mat->pixelShader = nullptr;

	fullPath = "../Shaders/" + name + "_VS.cso";
	if(ShaderUtilities::LoadShaderFromFile(fullPath, VSBytes))
	{
		foundVS = true;
	}
	else
		mat->vertexShader = nullptr;

	if(foundVS)
	{
		HRESULT hr = m_Device->CreateVertexShader(VSBytes.data(), VSBytes.size(), nullptr, &mat->vertexShader);
		if(SUCCEEDED(hr))
		{
			D3D11_INPUT_ELEMENT_DESC vLayout[] =
			{
					{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
					{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
					{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
					{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
					{ "BINORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
			};

			hr = m_Device->CreateInputLayout(vLayout, ARRAYSIZE(vLayout), VSBytes.data(), VSBytes.size(), &mat->inputLayout);

			if(FAILED(hr))
				foundVS = false;
		}
		else
			foundVS = false;
	}

	if(foundPS)
	{
		HRESULT hr = m_Device->CreatePixelShader(PSBytes.data(), PSBytes.size(), nullptr, &mat->pixelShader);
		if(FAILED(hr))
			foundPS = false;
	}

	if(foundPS || foundVS)
		return mat;

	mat->Release();
	GetResourceManager()->RemoveResource(mat, name);
	return nullptr;
}

Texture2D * Renderer::CreateTextureFromFile(std::string name)
{
	std::wstring str = L"../Textures/" + StringUtility::utf8_decode(name) + L".dds";

	Texture2D* texture = GetResourceManager()->CreateResource<Texture2D>(name);
	HRESULT hr = DirectX::CreateDDSTextureFromFile(m_Device.Get(), str.c_str(), &texture->d3dresource, &texture->shaderResourceView);
	if(SUCCEEDED(hr))
		return texture;

	texture->Release();
	GetResourceManager()->RemoveResource(texture, name);
	return nullptr;
}

void Renderer::DrawMesh(const Mesh* mesh, const XMMATRIX& transform)
{
	UINT stride = sizeof(Vertex);
	UINT offset = 0;

	CMatricesBuffer* matrices = static_cast<CMatricesBuffer*>(m_MatricesBuffer->cpu);

	matrices->WorldViewProjection = XMMatrixTranspose(transform*m_ViewProjection);
	XMMATRIX normalMatrix = transform;
	// Remove translation component
	normalMatrix.r[3].m128_f32[0] = normalMatrix.r[3].m128_f32[1] = normalMatrix.r[3].m128_f32[2] = 0;
	normalMatrix.r[3].m128_f32[3] = 1;
	normalMatrix = XMMatrixInverse(nullptr, normalMatrix);
	matrices->Normal = XMMatrixTranspose(normalMatrix);
	matrices->World = XMMatrixTranspose(transform);
	UpdateConstantBuffer(m_MatricesBuffer);

	m_Context->IASetVertexBuffers(0, 1, &mesh->geometry->vertexBuffer.data, &stride, &offset);
	m_Context->IASetIndexBuffer(mesh->geometry->indexBuffer.data, DXGI_FORMAT_R32_UINT, 0);
	m_Context->IASetInputLayout(mesh->material->inputLayout);
	m_Context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	m_Context->VSSetShader(mesh->material->vertexShader, nullptr, 0);
	m_Context->VSSetConstantBuffers(0, 1, &m_MatricesBuffer->gpu.data);
	m_Context->PSSetShader(mesh->material->pixelShader, nullptr, 0);
	m_Context->PSSetConstantBuffers(0, 1, &m_LightBuffer->gpu.data);
	m_Context->PSSetSamplers(0, 1, m_SamplerLinearWrap.GetAddressOf());
	m_Context->PSSetSamplers(1, 1, m_ShadowSampler.GetAddressOf());
	int i = 0;
	for(auto& tex : mesh->material->textures)
	{
		m_Context->PSSetShaderResources(i, 1, &mesh->material->textures[i]->shaderResourceView);
		++i;
	}
	m_Context->PSSetShaderResources(i, 1, &m_ShadowMap->shaderResourceView);

	m_Context->DrawIndexed(mesh->geometry->indexBuffer.size, 0, 0);
}

void Renderer::SetDirectionalLight(DirectionalLightComponent * light)
{
	m_DirectionalLight = light;

	InitializeShadowMaps(light->GetShadowResolution());

}

void Renderer::RenderShadowMaps()
{
	CreateRasterizerStates();
	CLightBuffer* buffer = static_cast<CLightBuffer*>(m_LightBuffer->cpu);
	m_ViewProjection = m_DirectionalLight->GetLightSpaceMatrix(m_ActiveCamera);
	m_Context->RSSetState(m_ShadowsRasterizerState.Get());
	RenderSceneToTexture(m_ShadowMap, m_DepthMaterial);
}

void Renderer::InitializeDefaultShaders()
{
	m_DepthMaterial = CreateMaterialFromFile("Depth");
}

void Renderer::InitializeConstantBuffers()
{
	m_MatricesBuffer = CreateConstantBuffer(sizeof(CMatricesBuffer), "CMatricesBuffer");
	m_LightBuffer = CreateConstantBuffer(sizeof(CLightBuffer), "LightBuffer");
}

void Renderer::InitializeShadowMaps(int resolution)
{
	HRESULT hr;
	// Create depth stencil texture
	m_ShadowMap = GetResourceManager()->CreateResource<RenderTexture2D>("ShadowMap");

	if(m_ShadowMap)
	{
		D3D11_TEXTURE2D_DESC textureDesc{};
		// Setup the render target texture description.
		textureDesc.Width = resolution;
		textureDesc.Height = resolution;
		textureDesc.MipLevels = 1;
		textureDesc.ArraySize = 1;
		textureDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		textureDesc.SampleDesc.Count = 1;
		textureDesc.Usage = D3D11_USAGE_DEFAULT;
		textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
		textureDesc.CPUAccessFlags = 0;
		textureDesc.MiscFlags = 0;

		// Create the render target texture.
		hr = m_Device->CreateTexture2D(&textureDesc, NULL, &m_ShadowMap->d3dtexture);

		if(SUCCEEDED(hr))
		{
			D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc{};
			renderTargetViewDesc.Format = textureDesc.Format;
			renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
			renderTargetViewDesc.Texture2D.MipSlice = 0;

			hr = m_Device->CreateRenderTargetView(m_ShadowMap->d3dtexture, &renderTargetViewDesc, &m_ShadowMap->renderTargetView);

			if(SUCCEEDED(hr))
			{
				D3D11_TEXTURE2D_DESC descDepth{};
				descDepth.Width = static_cast<UINT>(resolution);
				descDepth.Height = static_cast<UINT>(resolution);
				descDepth.MipLevels = 1;
				descDepth.ArraySize = 1;
				descDepth.Format = DXGI_FORMAT_R32_TYPELESS;
				descDepth.SampleDesc.Count = 1;
				descDepth.SampleDesc.Quality = 0;
				descDepth.Usage = D3D11_USAGE_DEFAULT;
				descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
				descDepth.CPUAccessFlags = 0;
				descDepth.MiscFlags = 0;
				hr = m_Device->CreateTexture2D(&descDepth, nullptr, &m_ShadowMap->d3dstencil);
				if(SUCCEEDED(hr))
				{
					D3D11_SHADER_RESOURCE_VIEW_DESC viewDesc{};
					viewDesc.Format = DXGI_FORMAT_R32_FLOAT;
					viewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
					viewDesc.Texture2D.MipLevels = 1;

					hr = m_Device->CreateShaderResourceView(m_ShadowMap->d3dstencil, &viewDesc, &m_ShadowMap->shaderResourceView);
					if(SUCCEEDED(hr))
					{
						// Create the depth stencil view
						D3D11_DEPTH_STENCIL_VIEW_DESC descDSV{};
						descDSV.Format = DXGI_FORMAT_D32_FLOAT;
						descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
						descDSV.Texture2D.MipSlice = 0;
						hr = m_Device->CreateDepthStencilView(m_ShadowMap->d3dstencil, &descDSV, &m_ShadowMap->depthStencilView);
						if(SUCCEEDED(hr))
						{
							m_DirectionalLightViewport = new D3D11_VIEWPORT{};
							m_DirectionalLightViewport->TopLeftX = 0;
							m_DirectionalLightViewport->TopLeftY = 0;

							m_DirectionalLightViewport->Width = resolution;
							m_DirectionalLightViewport->Height = resolution;

							m_DirectionalLightViewport->MaxDepth = 1.0f;
							m_DirectionalLightViewport->MinDepth = 0.0f;

							return;
						}
					}
				}
			}
		}
		m_ShadowMap->Release();
		GetResourceManager()->RemoveResource(m_ShadowMap, "ShadowMap");
	}
}

void Renderer::CreateRasterizerStates()
{
	CD3D11_RASTERIZER_DESC desc(D3D11_FILL_SOLID, D3D11_CULL_BACK, FALSE, 0, 0.f, 0.f, TRUE, FALSE, FALSE, FALSE);

	m_Device->CreateRasterizerState(&desc, &m_SceneRasterizerState);
	desc.DepthClipEnable = FALSE;
	//desc.SlopeScaledDepthBias = 1.0;
	m_Device->CreateRasterizerState(&desc, &m_ShadowsRasterizerState);

}

void Renderer::UpdateLightBuffers(std::vector<PointLightComponent>* pointLights, std::vector<SpotLightComponent>* spotLights)
{
	CLightBuffer* buffer = static_cast<CLightBuffer*>(m_LightBuffer->cpu);
	{//Directional light
		XMVECTOR color = m_DirectionalLight->GetLightColor()*m_DirectionalLight->GetLightIntensity();
		XMStoreFloat3(&buffer->lightInfo.directionalLight.color, color);
		XMVECTOR vec = m_DirectionalLight->GetLightDirection();
		XMStoreFloat3(&buffer->lightInfo.directionalLight.direction, vec);

		buffer->lightInfo.directionalShadowInfo.viewProj = XMMatrixTranspose(m_DirectionalLight->GetLightSpaceMatrix(m_ActiveCamera));

		buffer->lightInfo.directionalShadowInfo.bias = m_DirectionalLight->GetShadowBias();
		buffer->lightInfo.directionalShadowInfo.normalOffset = m_DirectionalLight->GetNormalOffset();
		buffer->lightInfo.directionalShadowInfo.resolution = m_DirectionalLight->GetShadowResolution();
	}
	//Point lights
	buffer->lightInfo.pointLightCount = pointLights->size();
	for(int i = 0; i < buffer->lightInfo.pointLightCount; ++i)
	{
		XMVECTOR color = (*pointLights)[i].GetLightColor()*(*pointLights)[i].GetLightIntensity();
		XMStoreFloat3(&buffer->lightInfo.pointLights[i].color, color);
		XMStoreFloat3(&buffer->lightInfo.pointLights[i].position, (*pointLights)[i].GetPosition());
		buffer->lightInfo.pointLights[i].radius = (*pointLights)[i].GetRadius();
	}

	//Spot lights
	buffer->lightInfo.spotLightCount = spotLights->size();
	for(int i = 0; i < buffer->lightInfo.spotLightCount; ++i)
	{
		XMVECTOR color = (*spotLights)[i].GetLightColor()*(*spotLights)[i].GetLightIntensity();
		XMStoreFloat3(&buffer->lightInfo.spotLights[i].color, color);
		XMStoreFloat3(&buffer->lightInfo.spotLights[i].position, (*spotLights)[i].GetPosition());
		XMStoreFloat3(&buffer->lightInfo.spotLights[i].direction, (*spotLights)[i].GetLightDirection());
		buffer->lightInfo.spotLights[i].radius = (*spotLights)[i].GetRadius();
		buffer->lightInfo.spotLights[i].innerCone = cos(XMConvertToRadians((*spotLights)[i].GetInnerAngle()));
		buffer->lightInfo.spotLights[i].outerCone = cos(XMConvertToRadians((*spotLights)[i].GetOuterAngle()));
	}

	UpdateConstantBuffer(m_LightBuffer);
}



void Renderer::UpdateConstantBuffer(ConstantBuffer* buffer)
{
	D3D11_MAPPED_SUBRESOURCE mappedResource{};
	m_Context->Map(buffer->gpu.data, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	memcpy(mappedResource.pData, buffer->cpu, buffer->gpu.size);
	m_Context->Unmap(buffer->gpu.data, 0);
}

ConstantBuffer * Renderer::CreateConstantBuffer(uint32_t size, std::string name)
{
	HRESULT hr;

	ConstantBuffer* buffer = GetResourceManager()->CreateResource<ConstantBuffer>(name);
	buffer->gpu.size = size;

	D3D11_BUFFER_DESC bd{};
	bd.Usage = D3D11_USAGE_DYNAMIC;
	bd.ByteWidth = size;
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	hr = m_Device->CreateBuffer(&bd, nullptr, &buffer->gpu.data);
	if(SUCCEEDED(hr))
	{
		buffer->cpu = malloc(size);
		return buffer;
	}

	buffer->Release();
	GetResourceManager()->RemoveResource(buffer, name);
	return nullptr;
}

void Renderer::RenderSceneToTexture(RenderTexture2D * output, Material * mat)
{
	float color[4] = {0.0f, 0.0f, 0.0f, 1.0f};
	m_Context->ClearDepthStencilView(output->depthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);
	m_Context->ClearRenderTargetView(output->renderTargetView, color);
	m_Context->OMSetRenderTargets(1, &output->renderTargetView, output->depthStencilView);
	m_Context->RSSetViewports(1, m_DirectionalLightViewport);

	for(auto& model : *m_ActiveModels)
	{
		for(auto& mesh : model.GetMeshes())
		{
			XMMATRIX transform = model.GetTransformMatrix();

			UINT stride = sizeof(Vertex);
			UINT offset = 0;

			CMatricesBuffer* matrices = static_cast<CMatricesBuffer*>(m_MatricesBuffer->cpu);
			matrices->WorldViewProjection = XMMatrixTranspose(transform*m_ViewProjection);
			UpdateConstantBuffer(m_MatricesBuffer);

			m_Context->IASetVertexBuffers(0, 1, &mesh->geometry->vertexBuffer.data, &stride, &offset);
			m_Context->IASetIndexBuffer(mesh->geometry->indexBuffer.data, DXGI_FORMAT_R32_UINT, 0);
			m_Context->IASetInputLayout(mat->inputLayout);
			m_Context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

			m_Context->VSSetShader(mat->vertexShader, nullptr, 0);
			m_Context->VSSetConstantBuffers(0, 1, &m_MatricesBuffer->gpu.data);
			m_Context->PSSetShader(mat->pixelShader, nullptr, 0);

			m_Context->DrawIndexed(mesh->geometry->indexBuffer.size, 0, 0);
		}
	}

	m_Context->OMSetRenderTargets(1, &output->renderTargetView, nullptr);
}
