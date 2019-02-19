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
struct ID3D11DepthStencilState;
struct ID3D11Resource;
struct ID3D11ShaderResourceView;
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
struct DepthTexture2D;
struct RenderTexture2DAllMips;

class DirectionalLightComponent;
class PointLightComponent;
class SpotLightComponent;
class CameraComponent;
class MeshComponent;

struct Vertex;
struct D3D11_BUFFER_DESC;
struct D3D11_SUBRESOURCE_DATA;
struct SurfaceProperties;
struct ID3D11Texture2D;
struct D3D11_TEXTURE2D_DESC;

struct VertexShader;
struct PixelShader;

class Effect;

struct CascadeShadows
{
	int cascadeCount;
	DepthTexture2D* shadowMap;
	std::vector<DirectX::XMMATRIX> cascadeMatrices;
	std::vector<float> cascadeSplits;

	CascadeShadows()
	{
		shadowMap = nullptr;
	}
};


class Renderer
{

public:
	Renderer();
	~Renderer();

	friend class Window;
	friend class Tonemapper;

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

	VertexShader* CreateVertexShader(std::string path);
	VertexShader* CreateVertexShaderPostProcess(std::string path);
	PixelShader* CreatePixelShader(std::string path);
	Material* CreateMaterial(std::string name);

	Texture2D* CreateTextureFromFile(std::string path);
	ConstantBuffer* CreateConstantBuffer(uint32_t size, std::string name);
	ID3D11Buffer* CreateD3DBuffer(D3D11_BUFFER_DESC* desc, D3D11_SUBRESOURCE_DATA* InitData);

	inline void SetActiveCamera(CameraComponent* camera) { m_ActiveCamera = camera; };

	void RenderSceneToTexture(RenderTexture2D* output);
	void RenderDepthToTexture(ID3D11DepthStencilView* dsv);


	inline ResourceManager* GetResourceManager() { return m_ResourceManager; };

	inline void SetActiveModels(std::vector<MeshComponent>* models) { m_ActiveModels = models; };
	void SetDirectionalLight(DirectionalLightComponent* light);

	bool FullScreenModeSwitched();

	bool ResizeSwapChain();

	void SetRenderTargets(unsigned int num, ID3D11RenderTargetView** rtv, ID3D11DepthStencilView* dsv);
	void ClearRenderTarget();
	void SetFullscreenViewport(float multiplier);
	void SetPixelShader(ID3D11PixelShader* ps);
	void SetPixelShaderResource(unsigned int slot, ID3D11ShaderResourceView** resource);
	void SetPixelShaderConstantBuffer(unsigned int slot, ID3D11Buffer** resource);
	void DrawScreenQuad();
	void GenerateMips(ID3D11ShaderResourceView* tex);

	inline RenderTexture2D* GetSceneTexture() { return m_SceneTexture; };
	RenderTexture2DAllMips* CreateRenderTexture2DAllMips(D3D11_TEXTURE2D_DESC* desc, std::string name);
	RenderTexture2D* CreateRenderTexture2D(D3D11_TEXTURE2D_DESC * desc, std::string name);

	template<class T>
	void AddPostProcessingEffect()
	{
		static_assert(std::is_base_of<Effect, T>::value, "Template effect not derived from Effect");
		T* effect = new T;
		m_PostProcessChain.push_back(effect);
		effect->Initialize(this);
	}
private:
	bool m_Paused = true;

	bool InitializeSwapChain();
	bool CreateIntermediateSceneTexture(ID3D11Texture2D* backBuffer, ID3D11Texture2D* stencilTx);
	Window* m_Window;

	CascadeShadows m_CascadeShadows;

	void InitializePostProcessing();
	void RenderPostProcessing();
	void RenderShadowMaps();
	void InitializeDefaultShaders();
	void InitializeConstantBuffers();
	void InitializeShadowMaps(float resolution);
	void CreateRasterizerStates();
	void SetShaderResources(Material* mat);
	void RenderSkybox();

	Microsoft::WRL::ComPtr<IDXGISwapChain1>				m_Swapchain;		// ptr to swap chain
	Microsoft::WRL::ComPtr<ID3D11Device1>				m_Device;			// ptr to device
	Microsoft::WRL::ComPtr<ID3D11DeviceContext1>		m_Context;			// ptr to device context
	Microsoft::WRL::ComPtr<ID3D11SamplerState>			m_ShadowSampler;
	Microsoft::WRL::ComPtr<ID3D11SamplerState>			m_SamplerLinearWrap;
	Microsoft::WRL::ComPtr<ID3D11SamplerState>			m_SamplerLinearClamp;
	Microsoft::WRL::ComPtr<ID3D11SamplerState>			m_SamplerSky;
	Microsoft::WRL::ComPtr<ID3D11SamplerState>			m_SamplerNearest;

	Microsoft::WRL::ComPtr<ID3D11RasterizerState>		m_SceneRasterizerState;
	Microsoft::WRL::ComPtr<ID3D11RasterizerState>		m_ShadowsRasterizerState;
	Microsoft::WRL::ComPtr<ID3D11RasterizerState>		m_SkyRasterizerState;

	Microsoft::WRL::ComPtr<ID3D11DepthStencilState>		m_DepthStencilState;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilState>		m_DepthStencilSkyState;


	ConstantBuffer*										m_LightInfoBuffer;
	ConstantBuffer*										m_SceneInfoBuffer;
	ConstantBuffer*										m_TransformBuffer;
	ConstantBuffer*										m_MaterialSurfaceBuffer;

	CameraComponent*									m_ActiveCamera;
	std::vector<MeshComponent>*							m_ActiveModels;
	RenderTexture2D*									m_FinalOutputTexture;
	RenderTexture2D*									m_SceneTexture;
	DirectionalLightComponent*							m_DirectionalLight;

	D3D11_VIEWPORT*										m_ActiveCameraViewport;
	D3D11_VIEWPORT*										m_DirectionalLightViewport;
	ResourceManager*									m_ResourceManager;
	DirectX::XMMATRIX									m_ViewProjection;
	DirectX::XMMATRIX									m_View;

	VertexShader*										FullscreenQuadVS;

	std::vector<Effect*>								m_PostProcessChain;
};

