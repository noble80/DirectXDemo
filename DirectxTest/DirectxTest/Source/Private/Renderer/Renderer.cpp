#include "stdafx.h"
#include "Renderer\Renderer.h"

#include "Renderer/Effects/Effect.h"
#include "Renderer\Effects\Tonemapper.h"
#include "Renderer\Effects\Bloom.h"

#include "Renderer/Window.h"
#include "Renderer/ShaderUtilities.h"
#include "Renderer/ShaderBuffers.h"
#include "Renderer/ResourceManager.h"

#include "Renderer\Texture2D.h"
#include "Renderer\RenderTexture2D.h"
#include "Renderer\Material.h"
#include "Renderer\Mesh.h"
#include "Renderer\ShaderContainers.h"

#include "Engine\MeshComponent.h"
#include "Engine/DirectionalLightComponent.h"
#include "Engine/SpotLightComponent.h"
#include "Engine/PointLightComponent.h"
#include "Engine/CameraComponent.h"
#include "Importer/DDSTextureLoader.h"

#include "StringUtility.h"

#include "Math/Vertex.h"
#include "Renderer\DebugHelpers.h"

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
	m_FinalOutputTexture = nullptr;
	m_ActiveCamera = nullptr;
	m_SceneTexture = nullptr;
	m_ActiveCameraViewport = new D3D11_VIEWPORT{};
	m_CascadeShadows.cascadeCount = 3;
}


Renderer::~Renderer()
{
}

bool Renderer::Initialize(Window * window)
{
	HRESULT hr = S_OK;
	m_Window = window;

	m_ResourceManager = new ResourceManager;
	m_ResourceManager->Initialize();

	Vector2 dimensions = window->GetDimensions();

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
		//dxgiFactory->MakeWindowAssociation(window->GetHandle(), DXGI_MWA_NO_ALT_ENTER);

		if(FAILED(hr))
			return false;
	}
#pragma endregion SWAPCHAIN_CREATION

	InitializeSwapChain();

	D3D11_DEPTH_STENCIL_DESC depthStencilDesc{};
	// Set up the description of the stencil state.
	depthStencilDesc.DepthEnable = true;
	depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;

	depthStencilDesc.StencilEnable = true;
	depthStencilDesc.StencilReadMask = 0xFF;
	depthStencilDesc.StencilWriteMask = 0xFF;

	// Stencil operations if pixel is front-facing.
	depthStencilDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
	depthStencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	// Stencil operations if pixel is back-facing.
	depthStencilDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
	depthStencilDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	m_Device->CreateDepthStencilState(&depthStencilDesc, &m_DepthStencilState);

	depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
	m_Device->CreateDepthStencilState(&depthStencilDesc, &m_DepthStencilSkyState);

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

		sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
		sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
		sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
		sampDesc.MipLODBias = 0.0f;
		sampDesc.MaxAnisotropy = 1;
		sampDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
		sampDesc.BorderColor[0] = 0;
		sampDesc.BorderColor[1] = 0;
		sampDesc.BorderColor[2] = 0;
		sampDesc.BorderColor[3] = 0;
		sampDesc.MinLOD = 0;
		sampDesc.MaxLOD = D3D11_FLOAT32_MAX;

		hr = m_Device->CreateSamplerState(&sampDesc, &m_SamplerLinearClamp);
		if(FAILED(hr))
			return hr;

		sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
		sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
		sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
		sampDesc.ComparisonFunc = D3D11_COMPARISON_LESS_EQUAL;

		sampDesc.MinLOD = 0;
		sampDesc.MaxLOD = D3D11_FLOAT32_MAX;

		hr = m_Device->CreateSamplerState(&sampDesc, &m_SamplerSky);
		if(FAILED(hr))
			return hr;

		sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
		sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
		sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
		sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
		hr = m_Device->CreateSamplerState(&sampDesc, &m_SamplerNearest);
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

	m_Window->m_Renderer = this;

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

	for(auto& ptr : m_PostProcessChain)
	{
		delete ptr;
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
	m_Context->ClearRenderTargetView(m_FinalOutputTexture->renderTargetView, color);
	m_Context->ClearRenderTargetView(m_SceneTexture->renderTargetView, color);

	// Clear the depth buffer to 1.0 (max depth)
	m_Context->ClearDepthStencilView(m_FinalOutputTexture->depthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);
	m_Context->OMSetRenderTargets(1, &m_SceneTexture->renderTargetView, m_FinalOutputTexture->depthStencilView);
	m_Context->OMSetDepthStencilState(m_DepthStencilState.Get(), 1);
	SetFullscreenViewport(1.0f);

	m_View = m_ActiveCamera->GetViewMatrix();
	m_ViewProjection = m_View * m_ActiveCamera->GetOrtographicProjectionMatrix();

	m_Context->RSSetState(m_SceneRasterizerState.Get());

	return true;
}

bool Renderer::Draw()
{
	return false;
}



void Renderer::RenderFrame(void)
{
	UpdateLightBuffers();
	RenderShadowMaps(m_ActiveCamera);
	UpdateConstantBuffer(m_LightInfoBuffer);
	InitializeGeometryPass();
	m_Context->PSSetConstantBuffers(6, 1, &m_LightInfoBuffer->gpu.data);
	m_Context->PSSetShaderResources(6, 1, &m_CascadeShadows.shadowMap->resourceView);
	for(auto& model : *m_ActiveModels)
	{
		for(auto& mesh : model.GetMeshes())
		{
			DrawMesh(mesh, model.GetTransformMatrix());
		}
	}
	RenderSkybox();
	RenderPostProcessing();
	ID3D11ShaderResourceView* null = nullptr;
	m_Context->PSSetShaderResources(6, 1, &null);
}

bool Renderer::PresentFrame()
{
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
		buffer->vertexBuffer.data = CreateD3DBuffer(&bd, &InitData);
		if(buffer->vertexBuffer.data)
		{
			bd.Usage = D3D11_USAGE_DEFAULT;
			bd.ByteWidth = sizeof(uint32_t) * buffer->indexBuffer.size;
			bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
			bd.CPUAccessFlags = 0;
			InitData.pSysMem = indices.data();
			buffer->indexBuffer.data = CreateD3DBuffer(&bd, &InitData);

			if(buffer->indexBuffer.data)
				return buffer;
		}

		GetResourceManager()->RemoveResource(buffer);
	}

	return nullptr;
}



VertexShader * Renderer::CreateVertexShader(std::string name)
{
	std::vector<std::byte> VSBytes;

	VertexShader* vs = GetResourceManager()->CreateResource<VertexShader>(name);
	std::string fullPath = "../Shaders/" + name + "_VS.cso";
	if(vs)
	{
		if(ShaderUtilities::LoadShaderFromFile(fullPath, VSBytes))
		{
			HRESULT hr = m_Device->CreateVertexShader(VSBytes.data(), VSBytes.size(), nullptr, &vs->d3dShader);
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

				hr = m_Device->CreateInputLayout(vLayout, ARRAYSIZE(vLayout), VSBytes.data(), VSBytes.size(), &vs->inputLayout);

				if(SUCCEEDED(hr))
					return vs;
			}
		}
		GetResourceManager()->RemoveResource(vs);
	}

	return nullptr;
}

VertexShader * Renderer::CreateVertexShaderPostProcess(std::string name)
{
	std::vector<std::byte> VSBytes;

	VertexShader* vs = GetResourceManager()->CreateResource<VertexShader>(name);
	std::string fullPath = "../Shaders/" + name + "_VS.cso";
	if(vs)
	{
		if(ShaderUtilities::LoadShaderFromFile(fullPath, VSBytes))
		{
			HRESULT hr = m_Device->CreateVertexShader(VSBytes.data(), VSBytes.size(), nullptr, &vs->d3dShader);
			if(SUCCEEDED(hr))
			{
				return vs;
			}
		}
		GetResourceManager()->RemoveResource(vs);
	}

	return nullptr;
}

PixelShader * Renderer::CreatePixelShader(std::string name)
{
	std::vector<std::byte> PSBytes;
	PixelShader* ps = GetResourceManager()->CreateResource<PixelShader>(name);
	std::string fullPath = "../Shaders/" + name + "_PS.cso";

	if(ps)
	{
		if(ShaderUtilities::LoadShaderFromFile(fullPath, PSBytes))
		{
			HRESULT hr = m_Device->CreatePixelShader(PSBytes.data(), PSBytes.size(), nullptr, &ps->d3dShader);
			if(SUCCEEDED(hr))
				return ps;
		}
		GetResourceManager()->RemoveResource(ps);
	}
	return nullptr;
}



Material* Renderer::CreateMaterial(std::string name)
{
	Material* mat = GetResourceManager()->CreateResource<Material>(name);

	return mat;
}

Texture2D * Renderer::CreateTextureFromFile(std::string name)
{
	std::wstring str = L"../Textures/" + StringUtility::utf8_decode(name) + L".dds";

	Texture2D* texture = GetResourceManager()->CreateResource<Texture2D>(name);
	HRESULT hr = DirectX::CreateDDSTextureFromFile(m_Device.Get(), str.c_str(), &texture->texture, &texture->resourceView);
	if(SUCCEEDED(hr))
		return texture;

	GetResourceManager()->RemoveResource(texture);
	return nullptr;
}

void Renderer::DrawDebugShape(GeometryBuffer * shape, const DirectX::XMMATRIX & transform)
{
	assert(DebugHelpers::DebugMat != nullptr && shape != nullptr);

	m_Context->OMSetRenderTargets(1, &m_FinalOutputTexture->renderTargetView, nullptr);

	UINT stride = sizeof(Vertex);
	UINT offset = 0;

	CTransformBuffer* matrices = static_cast<CTransformBuffer*>(m_TransformBuffer->cpu);

	matrices->WorldViewProjection = XMMatrixTranspose(transform*m_ViewProjection);
	UpdateConstantBuffer(m_TransformBuffer);
	m_Context->IASetVertexBuffers(0, 1, &shape->vertexBuffer.data, &stride, &offset);
	m_Context->IASetInputLayout(DebugHelpers::DebugMat->vertexShader->inputLayout);
	m_Context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP);

	m_Context->VSSetShader(DebugHelpers::DebugMat->vertexShader->d3dShader, nullptr, 0);
	m_Context->VSSetConstantBuffers(0, 1, &m_TransformBuffer->gpu.data);
	m_Context->PSSetShader(DebugHelpers::DebugMat->pixelShader->d3dShader, nullptr, 0);

	m_Context->Draw(shape->vertexBuffer.size, 0);
}

void Renderer::DrawMesh(const Mesh* mesh, const XMMATRIX& transform)
{
	UINT stride = sizeof(Vertex);
	UINT offset = 0;

	CTransformBuffer* matrices = static_cast<CTransformBuffer*>(m_TransformBuffer->cpu);

	matrices->WorldViewProjection = XMMatrixTranspose(transform*m_ViewProjection);
	XMMATRIX normalMatrix = transform;
	// Remove translation component
	normalMatrix.r[3].m128_f32[0] = normalMatrix.r[3].m128_f32[1] = normalMatrix.r[3].m128_f32[2] = 0;
	normalMatrix.r[3].m128_f32[3] = 1;
	normalMatrix = XMMatrixInverse(nullptr, normalMatrix);
	matrices->Normal = XMMatrixTranspose(normalMatrix);
	matrices->World = XMMatrixTranspose(transform);
	UpdateConstantBuffer(m_TransformBuffer);
	UpdateMaterialSurfaceBuffer(&mesh->material->surfaceParameters);

	m_Context->IASetVertexBuffers(0, 1, &mesh->geometry->vertexBuffer.data, &stride, &offset);
	m_Context->IASetIndexBuffer(mesh->geometry->indexBuffer.data, DXGI_FORMAT_R32_UINT, 0);
	m_Context->IASetInputLayout(mesh->material->vertexShader->inputLayout);
	m_Context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	m_Context->VSSetShader(mesh->material->vertexShader->d3dShader, nullptr, 0);
	m_Context->VSSetConstantBuffers(0, 1, &m_TransformBuffer->gpu.data);
	m_Context->PSSetShader(mesh->material->pixelShader->d3dShader, nullptr, 0);
	m_Context->PSSetSamplers(0, 1, m_SamplerLinearWrap.GetAddressOf());
	m_Context->PSSetSamplers(1, 1, m_ShadowSampler.GetAddressOf());
	m_Context->PSSetSamplers(2, 1, m_SamplerLinearClamp.GetAddressOf());
	m_Context->PSSetSamplers(3, 1, m_SamplerNearest.GetAddressOf());
	SetShaderResources(mesh->material);

	m_Context->DrawIndexed(mesh->geometry->indexBuffer.size, 0, 0);
}

void Renderer::SetDirectionalLight(DirectionalLightComponent * light)
{
	m_DirectionalLight = light;
	InitializeShadowMaps(light->GetShadowResolution());
}

bool Renderer::FullScreenModeSwitched()
{
	static BOOL result = false;
	BOOL newResult;
	m_Swapchain->GetFullscreenState(&newResult, nullptr);

	if(result != newResult)
	{
		result = newResult;
		return true;
	}
	return false;
}

bool Renderer::ResizeSwapChain()
{
	Vector2 dimensions = m_Window->GetDimensions();

	if(dimensions.x <= 0.f || dimensions.y <= 0.f)
		return false;

	m_Context->OMSetRenderTargets(0, 0, 0);
	m_FinalOutputTexture->renderTargetView->Release();
	m_FinalOutputTexture->depthStencilView->Release();
	HRESULT hr = m_Swapchain->ResizeBuffers(0, 0, 0, DXGI_FORMAT_UNKNOWN, DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH);

	if(FAILED(hr))
		return false;

	if(!InitializeSwapChain())
		return false;

	if(m_ActiveCamera)
	{
		if(m_Window)
			m_ActiveCamera->UpdateAspectRatio(m_Window->GetDimensions());
	}

	return true;
}

void Renderer::SetRenderTargets(unsigned int num, ID3D11RenderTargetView ** rtv, ID3D11DepthStencilView * dsv)
{
	m_Context->OMSetRenderTargets(num, rtv, dsv);
}

void Renderer::ClearRenderTarget()
{
	m_Context->OMSetRenderTargets(0, 0, 0);
}

void Renderer::SetFullscreenViewport(float multiplier)
{
	Vector2 dimensions = m_Window->GetDimensions();
	m_ActiveCameraViewport->Width = dimensions.x*multiplier;
	m_ActiveCameraViewport->Height = dimensions.y*multiplier;
	m_Context->RSSetViewports(1, m_ActiveCameraViewport);
}

void Renderer::SetPixelShader(ID3D11PixelShader * ps)
{
	m_Context->PSSetShader(ps, nullptr, 0);
}

void Renderer::SetPixelShaderResource(unsigned int slot, ID3D11ShaderResourceView** resource)
{
	m_Context->PSSetShaderResources(slot, 1, resource);
}

void Renderer::SetPixelShaderConstantBuffer(unsigned int slot, ID3D11Buffer** resource)
{
	m_Context->PSSetConstantBuffers(slot, 1, resource);

}

void Renderer::DrawScreenQuad()
{
	m_Context->Draw(4, 0);
}

void Renderer::GenerateMips(ID3D11ShaderResourceView* tex)
{
	m_Context->GenerateMips(tex);
}

RenderTexture2D* Renderer::CreateRenderTexture2D(D3D11_TEXTURE2D_DESC * desc, std::string name, bool useDepthStencil)
{
	RenderTexture2D* tex = GetResourceManager()->GetResource<RenderTexture2D>(name);
	if(tex)
	{
		GetResourceManager()->RemoveResource(tex);
	}
	desc->BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
	desc->MipLevels = 1;
	desc->SampleDesc.Count = 1;
	desc->ArraySize = 1;
	tex = GetResourceManager()->CreateResource<RenderTexture2D>(name);
	tex->dimensions.x = desc->Width;
	tex->dimensions.y = desc->Height;

	ID3D11Texture2D** ptr = reinterpret_cast<ID3D11Texture2D**>(&tex->texture);
	HRESULT hr = m_Device->CreateTexture2D(desc, NULL, ptr);

	D3D11_RENDER_TARGET_VIEW_DESC rtvDesc{};
	rtvDesc.Format = desc->Format;
	rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	rtvDesc.Texture2D.MipSlice = 0;

	hr = m_Device->CreateRenderTargetView(tex->texture, &rtvDesc, &tex->renderTargetView);
	m_Device->CreateShaderResourceView(tex->texture, nullptr, &tex->resourceView);

	if(useDepthStencil)
	{
		ComPtr<ID3D11Texture2D> stencilTx;
		{
			// Create depth stencil texture
			D3D11_TEXTURE2D_DESC descDepth{};
			descDepth.Width = desc->Width;
			descDepth.Height = desc->Height;
			descDepth.MipLevels = 1;
			descDepth.ArraySize = 1;
			descDepth.Format = DXGI_FORMAT_R24G8_TYPELESS;
			descDepth.SampleDesc.Count = 1;
			descDepth.SampleDesc.Quality = 0;
			descDepth.Usage = D3D11_USAGE_DEFAULT;
			descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
			descDepth.CPUAccessFlags = 0;
			descDepth.MiscFlags = 0;
			hr = m_Device->CreateTexture2D(&descDepth, nullptr, &stencilTx);

			// Create the depth stencil view
			D3D11_DEPTH_STENCIL_VIEW_DESC descDSV{};
			descDSV.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
			descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
			descDSV.Texture2D.MipSlice = 0;
			hr = m_Device->CreateDepthStencilView(stencilTx.Get(), &descDSV, &tex->depthStencilView);
		}
	}

	return tex;
}

RenderTexture2DAllMips * Renderer::CreateRenderTexture2DAllMips(D3D11_TEXTURE2D_DESC * desc, std::string name)
{
	RenderTexture2DAllMips* tex = GetResourceManager()->GetResource<RenderTexture2DAllMips>(name);
	if(tex)
	{
		GetResourceManager()->RemoveResource(tex);
	}

	tex = GetResourceManager()->CreateResource<RenderTexture2DAllMips>(name);
	HRESULT hr = m_Device->CreateTexture2D(desc, NULL, &tex->texture);

	for(int i = 0; i < desc->MipLevels; ++i)
	{
		D3D11_RENDER_TARGET_VIEW_DESC rtvDesc{};
		rtvDesc.Format = desc->Format;
		rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
		rtvDesc.Texture2D.MipSlice = i;

		ID3D11RenderTargetView* rtv;
		hr = m_Device->CreateRenderTargetView(tex->texture, &rtvDesc, &rtv);
		if(SUCCEEDED(hr))
			tex->RTVs.push_back(rtv);
	}
	//D3D11_SHADER_RESOURCE_VIEW_DESC rvDesc{};
	//rvDesc.Format = desc->Format;
	//rvDesc.Texture2D.MipLevels = desc->MipLevels;
	//rvDesc.Texture2D.MostDetailedMip = 0;
	//rvDesc.
	m_Device->CreateShaderResourceView(tex->texture, nullptr, &tex->resourceView);

	return tex;
}

void Renderer::SetActiveLights(DirectX::XMFLOAT3 ambientColor, std::vector<PointLightComponent>* pointLights, std::vector<SpotLightComponent>* spotLights)
{
	m_PointLights = pointLights;
	m_SpotLights = spotLights;
	m_AmbientColor = ambientColor;
}

bool Renderer::InitializeSwapChain()
{
	if(!m_FinalOutputTexture)
		m_FinalOutputTexture = GetResourceManager()->CreateResource<RenderTexture2D>("FinalRender");



	HRESULT hr;
#pragma region BACKBUFFER_CREATION
	/* Backbuffer */
	ComPtr<ID3D11Texture2D> backBuffer;
	{
		// get the address of the back buffer
		hr = m_Swapchain->GetBuffer(
			0, // we are using one backbuffer so we only need the first one
			__uuidof(ID3D11Texture2D), // get ID of texture type
			&backBuffer);

		if(FAILED(hr))
			return false;

		// use the back buffer address to create the render target
		hr = m_Device->CreateRenderTargetView(backBuffer.Get(), NULL, &m_FinalOutputTexture->renderTargetView);

		if(FAILED(hr))
			return false;
	}
#pragma endregion BACKBUFFER_CREATION
	D3D11_TEXTURE2D_DESC desc;
	backBuffer->GetDesc(&desc);
	Vector2 dimensions;
	dimensions.x = desc.Width;
	dimensions.y = desc.Height;
	m_Window->m_Dimensions = dimensions;
#pragma region STENCIL_CREATION
	/* Stencil */
	ComPtr<ID3D11Texture2D> stencilTx;
	{
		// Create depth stencil texture
		D3D11_TEXTURE2D_DESC descDepth{};
		descDepth.Width = static_cast<UINT>(dimensions.x);
		descDepth.Height = static_cast<UINT>(dimensions.y);
		descDepth.MipLevels = 1;
		descDepth.ArraySize = 1;
		descDepth.Format = DXGI_FORMAT_R24G8_TYPELESS;
		descDepth.SampleDesc.Count = 1;
		descDepth.SampleDesc.Quality = 0;
		descDepth.Usage = D3D11_USAGE_DEFAULT;
		descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
		descDepth.CPUAccessFlags = 0;
		descDepth.MiscFlags = 0;
		hr = m_Device->CreateTexture2D(&descDepth, nullptr, &stencilTx);
		if(FAILED(hr))
			return hr;

		// Create the depth stencil view
		D3D11_DEPTH_STENCIL_VIEW_DESC descDSV{};
		descDSV.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		descDSV.Texture2D.MipSlice = 0;
		hr = m_Device->CreateDepthStencilView(stencilTx.Get(), &descDSV, &m_FinalOutputTexture->depthStencilView);
		if(FAILED(hr))
			return hr;
	#pragma endregion STENCIL_CREATION
	}


#pragma region VIEWPORT_CREATION
	/* Viewport */

	// Set the viewport
	m_ActiveCameraViewport->TopLeftX = 0;
	m_ActiveCameraViewport->TopLeftY = 0;

	m_ActiveCameraViewport->Width = dimensions.x;
	m_ActiveCameraViewport->Height = dimensions.y;

	m_ActiveCameraViewport->MaxDepth = 1.0f;
	m_ActiveCameraViewport->MinDepth = 0.0f;
#pragma endregion VIEWPORT_CREATION

	CreateIntermediateSceneTexture(backBuffer.Get(), stencilTx.Get());
	InitializePostProcessing();

	return true;
}

bool Renderer::CreateIntermediateSceneTexture(ID3D11Texture2D* backBuffer, ID3D11Texture2D* stencilTx)
{
	if(m_SceneTexture)
		m_SceneTexture->Release();
	else
		m_SceneTexture = GetResourceManager()->CreateResource<RenderTexture2D>("IntermediateScene");
	D3D11_TEXTURE2D_DESC textureDesc;
	backBuffer->GetDesc(&textureDesc);
	// Setup the render target texture description.

	textureDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
	// Create the render target texture.
	ID3D11Texture2D** ptr = reinterpret_cast<ID3D11Texture2D**>(&m_SceneTexture->texture);
	HRESULT hr = m_Device->CreateTexture2D(&textureDesc, NULL, ptr);

	if(SUCCEEDED(hr))
	{
		D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc{};
		renderTargetViewDesc.Format = textureDesc.Format;
		renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
		renderTargetViewDesc.Texture2D.MipSlice = 0;

		hr = m_Device->CreateRenderTargetView(m_SceneTexture->texture, &renderTargetViewDesc, &m_SceneTexture->renderTargetView);

		if(SUCCEEDED(hr))
		{
			D3D11_SHADER_RESOURCE_VIEW_DESC viewDesc{};
			viewDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
			viewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
			viewDesc.Texture2D.MipLevels = 1;

			hr = m_Device->CreateShaderResourceView(stencilTx, &viewDesc, &m_SceneTexture->depthView);
			if(FAILED(hr))
			{
				return false;
			}
			viewDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;

			hr = m_Device->CreateShaderResourceView(m_SceneTexture->texture, &viewDesc, &m_SceneTexture->resourceView);
			if(FAILED(hr))
			{
				return false;
			}

			return true;
		}
	}

	return false;
}

void Renderer::InitializePostProcessing()
{
	for(auto& ptr : m_PostProcessChain)
	{
		ptr->Release(this);
		delete ptr;
	}
	m_PostProcessChain.clear();

	AddPostProcessingEffect<Bloom>();
	AddPostProcessingEffect<Tonemapper>();
}

void Renderer::RenderPostProcessing()
{
	m_Context->RSSetState(m_SceneRasterizerState.Get());
	m_Context->OMSetDepthStencilState(m_DepthStencilState.Get(), 1);
	m_Context->PSSetSamplers(3, 1, m_SamplerNearest.GetAddressOf());
	m_Context->PSSetSamplers(2, 1, m_SamplerLinearClamp.GetAddressOf());
	m_Context->IASetInputLayout(nullptr);
	m_Context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	m_Context->VSSetShader(FullscreenQuadVS->d3dShader, nullptr, 0);

	RenderTexture2D* current = m_SceneTexture;

	for(auto& effect : m_PostProcessChain)
	{
		current = effect->RenderEffect(this, current);
	}
}

void Renderer::RenderShadowMaps(CameraComponent* camera)
{
	ID3D11ShaderResourceView* null[8] = {nullptr};

	m_Context->PSSetShaderResources(0, 8, null);

	CLightInfoBuffer* buffer = static_cast<CLightInfoBuffer*>(m_LightInfoBuffer->cpu);
	m_DirectionalLight->GetLightSpaceMatrices(camera, &m_CascadeShadows);
	for(int i = 0; i < m_CascadeShadows.cascadeCount; ++i)
	{
		m_ViewProjection = m_CascadeShadows.cascadeMatrices[i];
		m_Context->RSSetState(m_ShadowsRasterizerState.Get());
		RenderDepthToTexture(m_CascadeShadows.shadowMap->DSVs[i]);
	}

	{//Directional light
		XMVECTOR color = m_DirectionalLight->GetLightColor()*m_DirectionalLight->GetLightIntensity();
		XMStoreFloat3(&buffer->lightInfo.directionalLight.color, color);
		XMVECTOR vec = m_DirectionalLight->GetLightDirection();
		XMStoreFloat3(&buffer->lightInfo.directionalLight.direction, vec);

		for(int i = 0; i < m_CascadeShadows.cascadeCount; i++)
		{
			buffer->lightInfo.directionalShadowInfo.cascades[i].lightSpace = XMMatrixTranspose(m_CascadeShadows.cascadeMatrices[i]);
			buffer->lightInfo.directionalShadowInfo.cascades[i].cascadeSplit = m_CascadeShadows.cascadeSplits[i];
		}
		buffer->lightInfo.directionalShadowInfo.normalOffset = m_DirectionalLight->GetNormalOffset();
		buffer->lightInfo.directionalShadowInfo.bias = m_DirectionalLight->GetShadowBias();
		buffer->lightInfo.directionalShadowInfo.resolution = m_DirectionalLight->GetShadowResolution();
	}
}


void Renderer::InitializeDefaultShaders()
{
	FullscreenQuadVS = CreateVertexShaderPostProcess("FullscreenQuad");
}

void Renderer::InitializeConstantBuffers()
{
	m_TransformBuffer = CreateConstantBuffer(sizeof(CTransformBuffer), "TransformBuffer");
	m_LightInfoBuffer = CreateConstantBuffer(sizeof(CLightInfoBuffer), "LightInfoBuffer");
	m_SceneInfoBuffer = CreateConstantBuffer(sizeof(CSceneInfoBuffer), "SceneInfoBuffer");
	m_MaterialSurfaceBuffer = CreateConstantBuffer(sizeof(SurfaceProperties), "MaterialSurfaceBuffer");
}

void Renderer::InitializeShadowMaps(float resolution)
{
	HRESULT hr;
	// Create depth stencil texture
	m_CascadeShadows.shadowMap = GetResourceManager()->CreateResource<DepthTexture2D>("ShadowMap");

	if(m_CascadeShadows.shadowMap)
	{
		D3D11_TEXTURE2D_DESC textureDesc{};
		// Setup the render target texture description.
		textureDesc.Width = (UINT)resolution;
		textureDesc.Height = (UINT)resolution;
		textureDesc.MipLevels = 1;
		textureDesc.ArraySize = m_CascadeShadows.cascadeCount;
		textureDesc.Format = DXGI_FORMAT_R32_TYPELESS;
		textureDesc.SampleDesc.Count = 1;
		textureDesc.Usage = D3D11_USAGE_DEFAULT;
		textureDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
		textureDesc.CPUAccessFlags = 0;
		textureDesc.MiscFlags = 0;

		// Create the render target texture.
		hr = m_Device->CreateTexture2D(&textureDesc, NULL, &m_CascadeShadows.shadowMap->texture);

		for(int i = 0; i < m_CascadeShadows.cascadeCount; ++i)
		{
			D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
			dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
			dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DARRAY;
			dsvDesc.Texture2DArray.ArraySize = 1;
			dsvDesc.Texture2DArray.FirstArraySlice = i;
			dsvDesc.Texture2D.MipSlice = 0;

			ID3D11DepthStencilView* dsv;
			hr = m_Device->CreateDepthStencilView(m_CascadeShadows.shadowMap->texture, &dsvDesc, &dsv);
			if(SUCCEEDED(hr))
				m_CascadeShadows.shadowMap->DSVs.push_back(dsv);
		}

		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
		srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
		srvDesc.Texture2DArray.ArraySize = m_CascadeShadows.cascadeCount;
		srvDesc.Texture2DArray.FirstArraySlice = 0;
		srvDesc.Texture2DArray.MipLevels = 1;
		srvDesc.Texture2DArray.MostDetailedMip = 0;

		hr = m_Device->CreateShaderResourceView(m_CascadeShadows.shadowMap->texture, &srvDesc, &m_CascadeShadows.shadowMap->resourceView);

		m_DirectionalLightViewport = new D3D11_VIEWPORT;
		m_DirectionalLightViewport->Height = resolution;
		m_DirectionalLightViewport->Width = resolution;
		m_DirectionalLightViewport->MaxDepth = 1.0f;
		m_DirectionalLightViewport->MinDepth = 0.0f;
		m_DirectionalLightViewport->TopLeftX = 0.f;
		m_DirectionalLightViewport->TopLeftY = 0.f;

		if(SUCCEEDED(hr))
			return;
	}
	GetResourceManager()->RemoveResource(m_CascadeShadows.shadowMap);
}

void Renderer::CreateRasterizerStates()
{
	CD3D11_RASTERIZER_DESC desc(D3D11_FILL_SOLID, D3D11_CULL_BACK, FALSE, 0, 0.f, 0.f, TRUE, FALSE, FALSE, FALSE);

	m_Device->CreateRasterizerState(&desc, &m_SceneRasterizerState);
	desc.DepthClipEnable = FALSE;
	//desc.SlopeScaledDepthBias = 1.0;
	m_Device->CreateRasterizerState(&desc, &m_ShadowsRasterizerState);
	desc.DepthClipEnable = TRUE;
	desc.CullMode = D3D11_CULL_FRONT;
	//desc.FrontCounterClockwise = TRUE;
	m_Device->CreateRasterizerState(&desc, &m_SkyRasterizerState);

}

void Renderer::SetShaderResources(Material * mat)
{
	for(int i = 0; i < 6; i++)
	{
		if(mat->textures[i])
			m_Context->PSSetShaderResources(i, 1, &mat->textures[i]->resourceView);
	}
}

void Renderer::RenderSkybox(bool flipFaces)
{
	XMMATRIX transform = XMMatrixScaling(1000.0f, 1000.0f, 1000.0f)*XMMatrixTranslationFromVector(m_ActiveCamera->GetCameraPosition());
	if(flipFaces)
		m_Context->RSSetState(m_SkyRasterizerState.Get());
	else
		m_Context->RSSetState(m_SceneRasterizerState.Get());

	m_Context->OMSetDepthStencilState(m_DepthStencilSkyState.Get(), 0);
	Mesh* mesh = GetResourceManager()->GetResource<Mesh>("SkySphere");
	if(mesh)
	{
		UINT stride = sizeof(Vertex);
		UINT offset = 0;

		CTransformBuffer* matrices = static_cast<CTransformBuffer*>(m_TransformBuffer->cpu);
		matrices->WorldViewProjection = XMMatrixTranspose(transform*m_ViewProjection);
		matrices->World = XMMatrixTranspose(transform);
		UpdateConstantBuffer(m_TransformBuffer);
		UpdateMaterialSurfaceBuffer(&mesh->material->surfaceParameters);
		m_Context->IASetVertexBuffers(0, 1, &mesh->geometry->vertexBuffer.data, &stride, &offset);
		m_Context->IASetIndexBuffer(mesh->geometry->indexBuffer.data, DXGI_FORMAT_R32_UINT, 0);
		m_Context->IASetInputLayout(mesh->material->vertexShader->inputLayout);
		m_Context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		m_Context->VSSetShader(mesh->material->vertexShader->d3dShader, nullptr, 0);
		m_Context->VSSetConstantBuffers(0, 1, &m_TransformBuffer->gpu.data);
		m_Context->PSSetShader(mesh->material->pixelShader->d3dShader, nullptr, 0);
		m_Context->PSSetSamplers(3, 1, m_SamplerSky.GetAddressOf());
		SetShaderResources(mesh->material);

		m_Context->DrawIndexed(mesh->geometry->indexBuffer.size, 0, 0);
	}
}

void Renderer::UpdateLightBuffers()
{
	CLightInfoBuffer* buffer = static_cast<CLightInfoBuffer*>(m_LightInfoBuffer->cpu);
	//Point lights
	buffer->lightInfo.pointLightCount = m_PointLights->size();
	for(int i = 0; i < buffer->lightInfo.pointLightCount; ++i)
	{
		XMVECTOR color = (*m_PointLights)[i].GetLightColor()*(*m_PointLights)[i].GetLightIntensity();
		XMStoreFloat3(&buffer->lightInfo.pointLights[i].color, color);
		XMStoreFloat3(&buffer->lightInfo.pointLights[i].position, (*m_PointLights)[i].GetPosition());
		buffer->lightInfo.pointLights[i].radius = (*m_PointLights)[i].GetRadius();
	}

	//Spot lights
	buffer->lightInfo.spotLightCount = m_SpotLights->size();
	for(int i = 0; i < buffer->lightInfo.spotLightCount; ++i)
	{
		XMVECTOR color = (*m_SpotLights)[i].GetLightColor()*(*m_SpotLights)[i].GetLightIntensity();
		XMStoreFloat3(&buffer->lightInfo.spotLights[i].color, color);
		XMStoreFloat3(&buffer->lightInfo.spotLights[i].position, (*m_SpotLights)[i].GetPosition());
		XMStoreFloat3(&buffer->lightInfo.spotLights[i].direction, (*m_SpotLights)[i].GetLightDirection());
		buffer->lightInfo.spotLights[i].radius = (*m_SpotLights)[i].GetRadius();
		buffer->lightInfo.spotLights[i].innerCone = cos(XMConvertToRadians((*m_SpotLights)[i].GetInnerAngle()));
		buffer->lightInfo.spotLights[i].outerCone = cos(XMConvertToRadians((*m_SpotLights)[i].GetOuterAngle()));
	}
	buffer->lightInfo.ambientColor = m_AmbientColor;

}

void Renderer::UpdateSceneBuffer(float time)
{
	CSceneInfoBuffer* buffer = static_cast<CSceneInfoBuffer*>(m_SceneInfoBuffer->cpu);
	XMStoreFloat3(&buffer->eyePosition, m_ActiveCamera->GetCameraPosition());
	buffer->time = time;
	UpdateConstantBuffer(m_SceneInfoBuffer);
	m_Context->VSSetConstantBuffers(5, 1, &m_SceneInfoBuffer->gpu.data);
	m_Context->PSSetConstantBuffers(5, 1, &m_SceneInfoBuffer->gpu.data);
}

void Renderer::UpdateMaterialSurfaceBuffer(const SurfaceProperties* prop)
{

	SurfaceProperties* buffer = static_cast<SurfaceProperties*>(m_MaterialSurfaceBuffer->cpu);
	memcpy(buffer, prop, sizeof(SurfaceProperties));
	UpdateConstantBuffer(m_MaterialSurfaceBuffer);
	m_Context->PSSetConstantBuffers(4, 1, &m_MaterialSurfaceBuffer->gpu.data);
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

	GetResourceManager()->RemoveResource(buffer);
	return nullptr;
}

void Renderer::RenderSceneToTexture(RenderTexture2D * output, CameraComponent* camera)
{
	RenderShadowMaps(camera);
	UpdateConstantBuffer(m_LightInfoBuffer);
	m_Context->PSSetConstantBuffers(6, 1, &m_LightInfoBuffer->gpu.data);
	m_Context->PSSetShaderResources(6, 1, &m_CascadeShadows.shadowMap->resourceView);
	D3D11_TEXTURE2D_DESC desc;
	ID3D11Texture2D* tex = reinterpret_cast<ID3D11Texture2D*>(output->texture);
	tex->GetDesc(&desc);
	RenderTexture2D* back = CreateRenderTexture2D(&desc, "RenderSceneToTextureBack", true);

	float color[4] = {0.0f, 0.0f, 0.0f, 1.0f};
	m_Context->ClearDepthStencilView(back->depthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);
	m_Context->ClearRenderTargetView(back->renderTargetView, color);
	m_Context->OMSetRenderTargets(1, &back->renderTargetView, back->depthStencilView);
	D3D11_VIEWPORT viewport;
	viewport.Height = back->dimensions.y;
	viewport.Width = back->dimensions.x;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	viewport.TopLeftX = 0.0f;
	viewport.TopLeftY = 0.0f;
	XMMATRIX flip = XMMatrixScaling(1.0f, -1.0f, 1.0f);
	m_View = camera->GetViewMatrix()*flip;
	m_ViewProjection = camera->GetViewProjectionMatrix()*flip;
	m_Context->RSSetViewports(1, &viewport);
	m_Context->RSSetState(m_SkyRasterizerState.Get());

	for(auto& model : *m_ActiveModels)
	{
		for(auto& mesh : model.GetMeshes())
		{
			DrawMesh(mesh, model.GetTransformMatrix());
		}
	}
	RenderSkybox(false);
	m_Context->OMSetRenderTargets(0, 0, nullptr);

	m_Context->CopyResource(output->texture, back->texture);
	GetResourceManager()->RemoveResource(back);
}

void Renderer::RenderDepthToTexture(ID3D11DepthStencilView* dsv)
{
	ID3D11RenderTargetView* null[] = {nullptr};
	m_Context->OMSetRenderTargets(1, null, dsv);
	m_Context->ClearDepthStencilView(dsv, D3D11_CLEAR_DEPTH, 1.0f, 0);
	m_Context->RSSetViewports(1, m_DirectionalLightViewport);

	for(auto& model : *m_ActiveModels)
	{
		for(auto& mesh : model.GetMeshes())
		{
			XMMATRIX transform = model.GetTransformMatrix();

			UINT stride = sizeof(Vertex);
			UINT offset = 0;

			CTransformBuffer* matrices = static_cast<CTransformBuffer*>(m_TransformBuffer->cpu);
			matrices->WorldViewProjection = XMMatrixTranspose(transform*m_ViewProjection);
			XMMATRIX normalMatrix = transform;
			matrices->World = XMMatrixTranspose(transform);
			UpdateConstantBuffer(m_TransformBuffer);

			m_Context->IASetVertexBuffers(0, 1, &mesh->geometry->vertexBuffer.data, &stride, &offset);
			m_Context->IASetIndexBuffer(mesh->geometry->indexBuffer.data, DXGI_FORMAT_R32_UINT, 0);
			m_Context->IASetInputLayout(mesh->material->vertexShader->inputLayout);
			m_Context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

			m_Context->VSSetShader(mesh->material->vertexShader->d3dShader, nullptr, 0);
			m_Context->PSSetShader(nullptr, nullptr, 0);

			m_Context->DrawIndexed(mesh->geometry->indexBuffer.size, 0, 0);
		}
	}

	m_Context->OMSetRenderTargets(0, 0, 0);
}

ID3D11Buffer* Renderer::CreateD3DBuffer(D3D11_BUFFER_DESC* desc, D3D11_SUBRESOURCE_DATA* InitData)
{
	ID3D11Buffer* buffer;

	HRESULT hr = m_Device->CreateBuffer(desc, InitData, &buffer);

	if(SUCCEEDED(hr))
		return buffer;
	else
		return nullptr;
}