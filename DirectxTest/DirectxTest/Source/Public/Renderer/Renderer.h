#pragma once
#include <wrl/client.h>

struct IDXGISwapChain1;
struct ID3D11Device1;
struct ID3D11DeviceContext1;
struct ID3D11RenderTargetView;
struct ID3D11DepthStencilView;
struct ID3D11Buffer;
struct ID3D11VertexShader;
struct ID3D11PixelShader;
struct D3D11_VIEWPORT;
struct ID3D11InputLayout;
struct ID3D11SamplerState;
struct ID3D11RasterizerState;

struct CMatricesBuffer;

class ResourceManager;
class Window;
struct Material;
struct GeometryBuffer;
struct Mesh;
struct Model;
struct ConstantBuffer;
struct Texture2D;
struct RenderTexture2D;

class DirectionalLightComponent;
class PointLightComponent;
class SpotLightComponent;
class CameraComponent;
class ModelComponent;

struct Vertex;

class Renderer
{

public:
	Renderer();
	~Renderer();

	bool Initialize(Window* window);
	bool Update();
	bool Shutdown();
	bool InitializeGeometryPass();
	bool Draw();
	bool RenderFrame();

	GeometryBuffer* CreateGeometryBuffer(std::string name, std::vector<Vertex>* vertices, std::vector<uint32_t> indices);
	Material* CreateMaterialFromFile(std::string path);
	Texture2D* CreateTextureFromFile(std::string path);
	void DrawMesh(const Mesh* mesh, const DirectX::XMMATRIX& transform);
	void UpdateLightBuffers(std::vector<PointLightComponent>* pointLights, std::vector<SpotLightComponent>* spotLights);
	void UpdateConstantBuffer(ConstantBuffer* buffer);
	ConstantBuffer* CreateConstantBuffer(uint32_t size, std::string name);
	inline void SetActiveCamera(CameraComponent* camera) { m_ActiveCamera = camera; };
	void RenderSceneToTexture(RenderTexture2D* output, Material* mat);

	inline ResourceManager* GetResourceManager() { return m_ResourceManager; };

	inline void SetActiveModels(std::vector<ModelComponent>* models) { m_ActiveModels = models; };
	void SetDirectionalLight(DirectionalLightComponent* light);

	void RenderShadowMaps();
private:
	void InitializeDefaultShaders();
	void InitializeConstantBuffers();
	void InitializeShadowMaps(int resolution);
	void CreateRasterizerStates();

	Microsoft::WRL::ComPtr<IDXGISwapChain1>				m_Swapchain;		// ptr to swap chain
	Microsoft::WRL::ComPtr<ID3D11Device1>				m_Device;			// ptr to device
	Microsoft::WRL::ComPtr<ID3D11DeviceContext1>		m_Context;			// ptr to device context
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView>		m_RenderTargetView;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView>		m_DepthStencilView;
	Microsoft::WRL::ComPtr<ID3D11SamplerState>			m_ShadowSampler;
	Microsoft::WRL::ComPtr<ID3D11SamplerState>			m_SamplerLinearWrap;

	Microsoft::WRL::ComPtr<ID3D11RasterizerState>		m_SceneRasterizerState;
	Microsoft::WRL::ComPtr<ID3D11RasterizerState>		m_ShadowsRasterizerState;

	   
	ConstantBuffer*										m_LightBuffer;
	ConstantBuffer*										m_MatricesBuffer;
	CameraComponent*									m_ActiveCamera;
	std::vector<ModelComponent>*						m_ActiveModels;
	Material*											m_DepthMaterial;
	RenderTexture2D*									m_ShadowMap;
	DirectionalLightComponent*							m_DirectionalLight;

	D3D11_VIEWPORT*										m_ActiveCameraViewport;
	D3D11_VIEWPORT*										m_DirectionalLightViewport;
	ResourceManager*									m_ResourceManager;
	DirectX::XMMATRIX									m_ViewProjection;
	DirectX::XMMATRIX									m_View;
};

