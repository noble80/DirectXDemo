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

struct CTransformBuffer;

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
class MeshComponent;

struct Vertex;
struct D3D11_BUFFER_DESC;
struct D3D11_SUBRESOURCE_DATA;
struct SurfaceProperties;

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
	void RenderFrame();
	bool PresentFrame();

	void DrawDebugShape(GeometryBuffer* shape, const DirectX::XMMATRIX& transform);
	void DrawMesh(const Mesh* mesh, const DirectX::XMMATRIX& transform);

	void UpdateLightBuffers(DirectX::XMFLOAT3 ambientColor, std::vector<PointLightComponent>* pointLights, std::vector<SpotLightComponent>* spotLights);
	void UpdateSceneBuffer(float time);
	void UpdateMaterialSurfaceBuffer(const SurfaceProperties* prop);
	void UpdateConstantBuffer(ConstantBuffer* buffer);

	GeometryBuffer* CreateGeometryBuffer(std::string name, std::vector<Vertex>* vertices, std::vector<uint32_t> indices);
	Material* CreateMaterialFromFile(std::string path);
	Texture2D* CreateTextureFromFile(std::string path);
	ConstantBuffer* CreateConstantBuffer(uint32_t size, std::string name);
	ID3D11Buffer* CreateD3DBuffer(D3D11_BUFFER_DESC* desc, D3D11_SUBRESOURCE_DATA* InitData);

	inline void SetActiveCamera(CameraComponent* camera) { m_ActiveCamera = camera; };

	void RenderSceneToTexture(RenderTexture2D* output, bool NoPixelShader = false);


	inline ResourceManager* GetResourceManager() { return m_ResourceManager; };

	inline void SetActiveModels(std::vector<MeshComponent>* models) { m_ActiveModels = models; };
	void SetDirectionalLight(DirectionalLightComponent* light);

private:
	void RenderShadowMaps();
	void InitializeDefaultShaders();
	void InitializeConstantBuffers();
	void InitializeShadowMaps(float resolution);
	void CreateRasterizerStates();

	Microsoft::WRL::ComPtr<IDXGISwapChain1>				m_Swapchain;		// ptr to swap chain
	Microsoft::WRL::ComPtr<ID3D11Device1>				m_Device;			// ptr to device
	Microsoft::WRL::ComPtr<ID3D11DeviceContext1>		m_Context;			// ptr to device context
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView>		m_RenderTargetView;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView>		m_DepthStencilView;
	Microsoft::WRL::ComPtr<ID3D11SamplerState>			m_ShadowSampler;
	Microsoft::WRL::ComPtr<ID3D11SamplerState>			m_SamplerLinearWrap;
	Microsoft::WRL::ComPtr<ID3D11SamplerState>			m_SamplerLinearClamp;

	Microsoft::WRL::ComPtr<ID3D11RasterizerState>		m_SceneRasterizerState;
	Microsoft::WRL::ComPtr<ID3D11RasterizerState>		m_ShadowsRasterizerState;

	   
	ConstantBuffer*										m_LightInfoBuffer;
	ConstantBuffer*										m_SceneInfoBuffer;
	ConstantBuffer*										m_TransformBuffer;
	ConstantBuffer*										m_MaterialSurfaceBuffer;

	CameraComponent*									m_ActiveCamera;
	std::vector<MeshComponent>*						m_ActiveModels;
	RenderTexture2D*									m_ShadowMap;
	DirectionalLightComponent*							m_DirectionalLight;

	D3D11_VIEWPORT*										m_ActiveCameraViewport;
	D3D11_VIEWPORT*										m_DirectionalLightViewport;
	ResourceManager*									m_ResourceManager;
	DirectX::XMMATRIX									m_ViewProjection;
	DirectX::XMMATRIX									m_View;
};

